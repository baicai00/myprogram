
def gen(proto_file, output_type, output_dif):
    if output_type == 'cpp':
        #将proto文件生成cpp 
        #todo
    elif output_type == 'lua': #单引号的字符串与双引号的字符串有什么区别???
        #将proto文件生成lua
        #todo

#arg[0]为脚本路径+脚本名，arg[1]为要转换的类型，目前支持'cpp'、'lua'
if __name__ == '__main__':
