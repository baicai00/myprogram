#include "skynet.h"
#include "skynet_socket.h"
#include "databuffer.h"
#include "hashid.h"
#include "skynet_env.h"
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>


#define SUBTYPE_PROTOBUF 16  // PTYPE_PROTO
#define SUBTYPE_PLAIN_TEXT 19

#define SUBTYPE_RPC_CLIENT 17 // PTYPE_RPC_CLIENT
#define SUBTYPE_RPC_SERVER 18 // PTYPE_RPC_SERVER

#define BACKLOG 32
// tcp connection 
struct connection {
	int id;	// skynet_socket id
	uint32_t agent;			// agent句柄
	uint32_t client;		// client 句柄
	char remote_name[32];	
	struct databuffer buffer; // 数据缓冲区
};

struct gate {
	struct skynet_context *ctx;
	int listen_id;
	uint32_t watchdog;
	uint32_t broker;
	int client_tag;
	int header_size;
	int max_connection;
	struct hashid hash;
	struct connection *conn;
	// todo: save message pool ptr for release
	struct messagepool mp;

	char bind[200]; // 添加了bind 代表监听的地址 char * listen_addr
};

struct gate *
gate_create(void) {
	struct gate * g = skynet_malloc(sizeof(*g));
	memset(g,0,sizeof(*g));
	g->listen_id = -1;
	return g;
}

void
gate_release(struct gate *g) {
	int i;
	struct skynet_context *ctx = g->ctx;
	for (i=0;i<g->max_connection;i++) {
		struct connection *c = &g->conn[i];
		if (c->id >=0) {
			skynet_socket_close(ctx, c->id);
		}
	}
	if (g->listen_id >= 0) {
		skynet_socket_close(ctx, g->listen_id);
	}
	messagepool_free(&g->mp);
	hashid_clear(&g->hash);
	skynet_free(g->conn);
	skynet_free(g);
}

static void
_parm(char *msg, int sz, int command_sz) {
	while (command_sz < sz) {
		if (msg[command_sz] != ' ')
			break;
		++command_sz;
	}
	int i;
	for (i=command_sz;i<sz;i++) {
		msg[i-command_sz] = msg[i];
	}
	msg[i-command_sz] = '\0';
}

static void
_forward_agent(struct gate * g, int fd, uint32_t agentaddr, uint32_t clientaddr) {
	int id = hashid_lookup(&g->hash, fd);
	skynet_error(g->ctx, "_forward_agent id = %d", id);
	if (id >=0) {
		struct connection * agent = &g->conn[id];
		agent->agent = agentaddr;
		agent->client = clientaddr;
	}
}

static int
start_listen(struct gate *g, char * listen_addr) {
	struct skynet_context * ctx = g->ctx;
	char * portstr = strchr(listen_addr, ':');
	const char * host = "";
	int port;
	if (portstr == NULL) {
		port = strtol(listen_addr, NULL, 10);
		if (port <= 0) {
			skynet_error(ctx, "Invalid gate address %s", listen_addr);
			return 1;
		}
	}
	else {
		port = strtol(portstr + 1, NULL, 10);
		if (port <= 0) {
			skynet_error(ctx, "Invalid gate address %s", listen_addr);
			return 1;
		}
		portstr[0] = '\0';
		host = listen_addr;
	}
	g->listen_id = skynet_socket_listen(ctx, host, port, BACKLOG);
	if (g->listen_id < 0) {
		return 1;
	}
	skynet_socket_start(ctx, g->listen_id);
	skynet_error(ctx, "start listen address %s", listen_addr);
	return 0;
}

static void
_ctrl(struct gate * g, const void * msg, int sz, uint32_t source) {
	struct skynet_context * ctx = g->ctx;
	char tmp[sz+1];
	memcpy(tmp, msg, sz);
	tmp[sz] = '\0';
	char * command = tmp;
	int i;
	if (sz == 0)
		return;
	for (i=0;i<sz;i++) {
		if (command[i]==' ') {
			break;
		}
	}
	if (memcmp(command,"kick",i)==0) {
		
		_parm(tmp, sz, i);
		int uid = strtol(command , NULL, 10);
		int id = hashid_lookup(&g->hash, uid);
		if (id>=0) {
			skynet_socket_close(ctx, uid);
		}
		return;
	}
	// // 重点是forward指令 agent模式中，gate会将数据直接发给agent
	if (memcmp(command,"forward",i)==0) { //forward马上start forward 16 :c800001f :0
		skynet_error(ctx, "forward i  = %d", i); 
		_parm(tmp, sz, i);
		char * client = tmp;
		char * idstr = strsep(&client, " ");

		if (client == NULL) {
			return;
		}
		int id = strtol(idstr , NULL, 10);
		skynet_error(ctx, "forward id  = %d", id);
		char * agent = strsep(&client, " ");
		if (client == NULL) {
			return;
		}
		uint32_t agent_handle = strtoul(agent+1, NULL, 16);
		skynet_error(ctx, "forward agent_handle = %u", agent_handle);
		uint32_t client_handle = strtoul(client+1, NULL, 16);
		skynet_error(ctx, "forward client_handle = %u", client_handle);
		_forward_agent(g, id, agent_handle, client_handle);
		return;
	}
	if (memcmp(command,"broker",i)==0) {
		_parm(tmp, sz, i);
		g->broker = skynet_queryname(ctx, command);
		return;
	}
	if (memcmp(command,"start",i) == 0) {
		_parm(tmp, sz, i);
		int uid = strtol(command , NULL, 10);
		int id = hashid_lookup(&g->hash, uid);
		if (id>=0) {
			skynet_socket_start(ctx, uid);
		}
		return;
	}
	if (memcmp(command, "close", i) == 0) {
		if (g->listen_id >= 0) {
			skynet_socket_close(ctx, g->listen_id);
			g->listen_id = -1;
		}
		return;
	}
	// 添加的新逻辑connect ip port
	if (memcmp(command, "connect", i) == 0)
	{
		char* p = &command[i + 1];
		char ip[100];
		char port[50];
		int i;
		while (*p != 0 && *p == ' ')
			++p;
		i = 0;
		while (*p != 0 && *p != ' ')
			ip[i++] = *p++;
		ip[i] = 0;
		while (*p != 0 && *p == ' ')
			++p;
		i = 0;
		while (*p != 0 && *p != ' ')
			port[i++] = *p++;
		port[i] = 0;
		int iport = atoi(port);
		if (hashid_full(&g->hash)) {
			return;
		}
		else {
			int fd = skynet_socket_connect(ctx, ip, iport);
			hashid_insert(&g->hash, fd);
			_forward_agent(g, fd, source, 0);
		}
		skynet_error(ctx, "connect to %s port %d", ip, iport);
		return;
	}
	// 把listen提上来
	if (memcmp(command, "listen", i) == 0)
	{
		start_listen(g, g->bind);
		return;
	}

	skynet_error(ctx, "[gate] Unkown command : %s", command);
}

/*static void
_report(struct gate * g, const char * data, ...) {
	if (g->watchdog == 0) {
		return;
	}
	struct skynet_context * ctx = g->ctx;

	va_list ap;
	va_start(ap, data);
	char tmp[1024];
	int n = vsnprintf(tmp, sizeof(tmp), data, ap);
	va_end(ap);

	//skynet_error(ctx, "_report PTYPE_TEXT n  = %d, tmp = %s", n, tmp);
	// 转换tmp为二进制

	
	skynet_send(ctx, 0, g->watchdog, PTYPE_TEXT,  0, tmp, n); 
	
}*/

static void
_report(struct gate * g, const char * data, ...) {
	if (g->watchdog == 0) {
		return;
	}
	struct skynet_context * ctx = g->ctx;

	va_list ap;
	va_start(ap, data);
	char tmp[1024];
	int n = vsnprintf(tmp, sizeof(tmp), data, ap);
	va_end(ap);


	uint32_t size = sizeof(uint32_t) + n;

	char str[1024];

	uint32_t sub_type = htonl(19);
	memcpy(str, &sub_type, sizeof(uint32_t)); // sub_type

	memcpy(str + sizeof(uint32_t), tmp, n); // tmp

	skynet_send(ctx, 0, g->watchdog, PTYPE_TEXT, 0, str, size);

}


static void
_forward(struct gate *g, struct connection * c, int size) {
	struct skynet_context * ctx = g->ctx;
	if (g->broker) {
		void * temp = skynet_malloc(size);
		databuffer_read(&c->buffer,&g->mp,temp, size);
		skynet_send(ctx, 0, g->broker, g->client_tag | PTYPE_TAG_DONTCOPY, 0, temp, size);
		return;
	}
	if (c->agent) {
		// 分配缓冲区
		void * temp = skynet_malloc(size);
		// 在缓冲区中读取数据
		databuffer_read(&c->buffer,&g->mp,temp, size);
		// int skynet_send(struct skynet_context * context, uint32_t source, uint32_t destination , int type, int session, void * msg, size_t sz);
		// c->client send message to c->agent 发送PTYPE_CLIENT类型的协议
		
		//skynet_error(ctx, "gate send to agent g->client_tag = %d", g->client_tag);
		skynet_send(ctx, c->client, c->agent, g->client_tag | PTYPE_TAG_DONTCOPY, 0 , temp, size);
	} else if (g->watchdog) {
		char * tmp = skynet_malloc(size + 32);
		int n = snprintf(tmp,32,"%d ",c->id);
		databuffer_read(&c->buffer,&g->mp,tmp+n,size);
		// 由gate发送到watchdog, 协议类型是PTYPE_CLIENT 大小为size + 32byte的数据
		
		//skynet_error(ctx, "gate send to watchdog g->client_tag = %d", g->client_tag);
		skynet_send(ctx, 0, g->watchdog, g->client_tag | PTYPE_TAG_DONTCOPY, 0, tmp, size + n);
	}
}

static void
dispatch_message(struct gate *g, struct connection *c, int id, void * data, int sz) {
	databuffer_push(&c->buffer,&g->mp, data, sz);
	for (;;) {
		int size = databuffer_readheader(&c->buffer, &g->mp, g->header_size);
		//skynet_error(NULL, "databuffer read return:%d buffersize:%d packsize:%d", size, c->buffer.size, c->buffer.header);
		if (size < 0) {
			return;
		} else if (size > 0) {
			if (size >= 0x1000000) {
				struct skynet_context * ctx = g->ctx;
				databuffer_clear(&c->buffer,&g->mp);
				skynet_socket_close(ctx, id);
				skynet_error(ctx, "Recv socket message > 16M");
				return;
			} else {
				//  接受socket数据，并转发到(watchdog, agent, broker)
				//skynet_error(NULL, "Accept socket and send to watchdog, agent, broker");
				_forward(g, c, size);
				databuffer_reset(&c->buffer);
			}
		}
	}
}

static void
dispatch_socket_message(struct gate *g, const struct skynet_socket_message * message, int sz) {
	//skynet_error(NULL, "gate get a socket msg size:%d type:%d", sz, message->type);
	struct skynet_context * ctx = g->ctx;
	
//	skynet_error(ctx, "dispatch_socket_message message->type = %d", message->type);

	switch(message->type) {
	case SKYNET_SOCKET_TYPE_DATA: {
		int id = hashid_lookup(&g->hash, message->id);
		//skynet_error(ctx, "SKYNET_SOCKET_TYPE_DATA id = %d, message->id = %d", id, message->id);
		if (id>=0) {
			struct connection *c = &g->conn[id];
			dispatch_message(g, c, message->id, message->buffer, message->ud);
		} else {
			//skynet_error(ctx, "Drop unknown connection %d message", message->id);
			skynet_socket_close(ctx, message->id);
			skynet_free(message->buffer);
		}
		break;
	}
	case SKYNET_SOCKET_TYPE_CONNECT: {  //accept -> forward+open -> connect
		if (message->id == g->listen_id) {
			// start listening
			skynet_error(NULL, "listen ok");
			break;
		}
		int id = hashid_lookup(&g->hash, message->id);
		if (id<0) {
			//skynet_error(ctx, "Close unknown connection %d", message->id);
			skynet_socket_close(ctx, message->id);
		}
		else
		{
			struct connection *c = &g->conn[id];
			char sendline[100];
			snprintf(sendline, sizeof(sendline), "%u connected %d", htonl(19), message->id);
			//skynet_error(ctx, "SKYNET_SOCKET_TYPE_CONNECT PTYPE_TEXT");

			skynet_send(ctx, 0, c->agent, PTYPE_TEXT, 0, sendline, strlen(sendline));
		}
		break;
	}
	case SKYNET_SOCKET_TYPE_CLOSE:
	case SKYNET_SOCKET_TYPE_ERROR: {
		int id = hashid_remove(&g->hash, message->id);
		if (id>=0) {
			struct connection *c = &g->conn[id];
			uint32_t agent = c->agent;
			skynet_error(ctx, "socket close: %d, msg id:%d", c->id, message->id);
			databuffer_clear(&c->buffer,&g->mp);
			memset(c, 0, sizeof(*c));
			c->id = -1;

			_report(g, "gate close %d %u", message->id, agent);
		}
		break;
	}
	case SKYNET_SOCKET_TYPE_ACCEPT:
		// report accept, then it will be get a SKYNET_SOCKET_TYPE_CONNECT message
		assert(g->listen_id == message->id);
		if (hashid_full(&g->hash)) {
			skynet_socket_close(ctx, message->ud);
		} else {
			struct connection *c = &g->conn[hashid_insert(&g->hash, message->ud)];
			if (sz >= sizeof(c->remote_name)) {
				sz = sizeof(c->remote_name) - 1;
			}
			c->id = message->ud;
			memcpy(c->remote_name, message+1, sz);
			c->remote_name[sz] = '\0';
			
			_report(g, "gate accept %d %s", c->id, c->remote_name);
			skynet_error(ctx, "socket open: %d", c->id);
		}
		break;
	case SKYNET_SOCKET_TYPE_WARNING:
		skynet_error(ctx, "fd (%d) send buffer (%d)K", message->id, message->ud);
		break;
	}
}

// 回调函数接受三种类型的消息
// PTYPE_TEXT: 控制信息 服务内部的指令
// PTYPE_CLIENT: 客户端发来的消息
// PTYPE_SOCKET: socket消息
static int
_cb(struct skynet_context * ctx, void * ud, int type, int session, uint32_t source, const void * msg, size_t sz) {
	struct gate *g = ud;
	//skynet_error(NULL, "gate msg size:%d type:%d", sz, type);
	switch(type) {
	case PTYPE_TEXT:
		// kick forward broker start close connect listen
		//skynet_error(ctx, "GATE PTYPE_TEXT");
		_ctrl(g , msg , (int)sz, source);
		break;
	case PTYPE_CLIENT: {
		if (sz <=4 ) {
			skynet_error(ctx, "Invalid client message from %x",source);
			break;
		}
		//skynet_error(ctx, "GATE PTYPE_CLIENT");
		// The last 4 bytes in msg are the id of socket, write following bytes to it
		const uint8_t * idbuf = msg + sz - 4;
		uint32_t uid = idbuf[0] | idbuf[1] << 8 | idbuf[2] << 16 | idbuf[3] << 24;
		skynet_error(ctx, "uid is %d", (int)uid);
		int id = hashid_lookup(&g->hash, uid);
		if (id>=0) {
			// don't send id (last 4 bytes)
			skynet_error(ctx, "gate accept client msg uid = %d", (int)uid);
			skynet_socket_send(ctx, uid, (void*)msg, sz-4); // 给watchdog发送消息
			// return 1 means don't free msg
			return 1;
		} else {
			skynet_error(ctx, "Invalid client id %d from %x",(int)uid,source);
			break;
		}
	}
	case PTYPE_SOCKET:
		// recv socket message from skynet_socket
		// gate首先接受socket消息
		//skynet_error(ctx, "GATE PTYPE_SOCKET");
		dispatch_socket_message(g, msg, (int)(sz-sizeof(struct skynet_socket_message)));
		break;
	}
	return 0;
}


int
gate_init(struct gate *g , struct skynet_context * ctx, char * parm) {
	if (parm == NULL)
		return 1;
	int sz = strlen(parm)+1;
	unsigned watchdog;
	char binding[sz];
	sscanf(parm, "%s %u", binding, &watchdog);  //passed through watchdog
	g->watchdog = watchdog;
	g->ctx = ctx;
	int max = atoi(skynet_getenv("max_online"));
	hashid_init(&g->hash, max);
	g->conn = skynet_malloc(max * sizeof(struct connection));
	memset(g->conn, 0, max *sizeof(struct connection));
	g->max_connection = max;
	int i;
	for (i=0;i<max;i++) {
		g->conn[i].id = -1;
	}
	
	g->client_tag = PTYPE_CLIENT;
	g->header_size = 4;

	skynet_callback(ctx,g,_cb);

	if (binding[0] == '!')
		return 0;

	memcpy(g->bind, binding, sz);

	return start_listen(g,binding);
	//return 0;
}
