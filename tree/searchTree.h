// 二叉搜索树

#ifndef _SEARCH_TREE_H_
#define _SEARCH_TREE_H_

#include <stdio.h>

// 二叉树Key的类型
typedef int TKey;

// 二叉搜索树的节点
typedef struct tagSearchTreeNode
{
    TKey                key;
    STSearchTreeNode    *parent;
    STSearchTreeNode    *left;
    STSearchTreeNode    *right;
} STSearchTreeNode;

// 二叉搜索树
class CBinarySearchTree
{
public:
    CBinarySearchTree();
    ~CBinarySearchTree();

    // 中序遍历二叉搜索树
    void inorder_tree_walk(TKey*& pOut, int& nLen);

    // 插入一个节点
    bool insertNode(TKey key);

    // 删除指定的节点
    void deleteNode(TKey key);

    // 返回树的最小节点
    bool minKey(TKey& key);

    // 返回树的最大节点
    bool maxKey(TKey& key);

    // 查找指定的节点
    STSearchTreeNode* search(TKey key);

    // 寻找key的后继节点
    STSearchTreeNode* treeSuccessor(TKey key);

    // 寻找key的前驱节点
    STSearchTreeNode* treePreDecessor(TKey key);

private:
    STSearchTreeNode* createNode(TKey key);
    bool insert(STSearchTreeNode* pNode);

    void inorder_tree_walk(STSearchTreeNode* pNode, TKey* pOut, int& nPos);

    // 用一棵以v为根的子树来替换一棵以u为根的子树
    void transplant(STSearchTreeNode* u, STSearchTreeNode* v);

private:
    STSearchTreeNode*       m_pRoot;
    int                     m_nLen;
};

#endif