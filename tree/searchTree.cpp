#include "searchTree.h"

CBinarySearchTree::CBinarySearchTree()
{
    m_pRoot = NULL;
    m_nLen = 0;
}

CBinarySearchTree::~CBinarySearchTree()
{
    if (NULL != m_pRoot)
    {
        delete m_pRoot;
        m_pRoot = NULL;
    }
    m_nLen = 0;
}

void CBinarySearchTree::inorder_tree_walk(TKey*& pOut, int& nLen)
{
    if (NULL == m_pRoot)
    {
        return;
    }
    if (NULL != pOut)
    {
        delete[] pOut;
    }
    pOut = new TKey[m_nLen];
    nLen = m_nLen;
    
    STSearchTreeNode* pNode = m_pRoot;
    int nPos = 0;
    inorder_tree_walk(pNode, pOut, nPos);
}

bool CBinarySearchTree::insertNode(TKey key)
{
    STSearchTreeNode* pNode = createNode(key);
    if (NULL == pNode)
    {
        return false;
    }

    return insert(pNode);
}

void CBinarySearchTree::deleteNode(TKey key)
{
    // 假如将要被删除的节点为x
    // 若x无子节点，直接删除
    STSearchTreeNode* x = search(key);
    if (NULL == x)
    {
        return;
    }
    
    if (x->left == NULL)
    {
        // x无左结点，则用x的右节点替代x
        transplant(x, x->right);
    }
    else if (x->right == NULL)
    {
        // x无右结点，则用x的左节点替代x
        transplant(x, x->left);
    }
    else
    {
        // x拥有左、右节点
        STSearchTreeNode* y = treeSuccessor(key); // 寻找x的后继节点
        if (y->parent != x)
        {
            // x的后继节点不是x的右儿子的情况
            // 先用y的右儿子替代y，因为后继节点无左儿子
            transplant(y, y->right);
            y->right = x->right;
            y->right->parent = y;
        }
        transplant(x, y);
        y->left = x->left;
        y->left->parent = y;
    }
}

bool CBinarySearchTree::minKey(TKey& key)
{
    if (NULL == m_pRoot)
    {
        return false;
    }

    STSearchTreeNode* pNode = m_pRoot;
    while (pNode->left)
    {
        pNode = pNode->left;
    }
    key = pNode->left->key;

    return true;
}

bool CBinarySearchTree::maxKey(TKey& key)
{
    if (NULL == m_pRoot)
    {
        return false;
    }

    STSearchTreeNode* pNode = m_pRoot;
    while (pNode->right)
    {
        pNode = pNode->right;
    }
    key = pNode->right->key;

    return true;
}

STSearchTreeNode* CBinarySearchTree::search(TKey key)
{
    if (NULL == m_pRoot)
    {
        return NULL;
    }

    STSearchTreeNode* pNode = m_pRoot;
    while (pNode)
    {
        if (pNode->key == key)
        {
            return pNode;
        }
        else if (pNode->key > key)
        {
            pNode = pNode->left;
        }
        else
        {
            pNode = pNode->right;
        }
    }
    return NULL;
}

STSearchTreeNode* CBinarySearchTree::treeSuccessor(TKey key)
{
    STSearchTreeNode *pNode = search(key);
    if (NULL == pNode)
    {
        return NULL;
    }

    // 节点有右儿子，则后继节点为节点右子树中的最小节点
    if (NULL != pNode->right)
    {
        STSearchTreeNode* pTmp = pNode->right;
        while (pTmp->left)
        {
            pTmp = pTmp->left;
        }
        return pTmp;
    }

    // 节点无右儿子，则节点的后继节点为其最低祖父节点y，且y的左儿子也为该节点的祖父节点
    STSearchTreeNode* y = pNode->parent;
    while (NULL != y && pNode == y->right)
    {
        pNode = y;
        y = y->parent;
    }
    return y;
}

STSearchTreeNode* CBinarySearchTree::treePreDecessor(TKey key)
{
    STSearchTreeNode *pNode = search(key);
    if (NULL == pNode)
    {
        return NULL;
    }

    // 节点左儿子存在，则节点的前驱节点为左子树中的最大节点
    if (NULL != pNode->left)
    {
        STSearchTreeNode* pTmp = pNode->left;
        while (pTmp->right)
        {
            pTmp = pTmp->right;
        }
        return pTmp;
    }

    // 节点左儿子不存在，则节点的前驱节点为其最你祖父节点y，且y的右儿子也为该节点的祖父节点
    STSearchTreeNode* y = pNode->parent;
    while (NULL != y && pNode == y->left)
    {
        pNode = y;
        y = y->parent;
    }
    return y;
}

STSearchTreeNode* CBinarySearchTree::createNode(TKey key)
{
    STSearchTreeNode* pNode = new STSearchTreeNode;
    if (NULL == pNode)
    {
        return;
    }
    pNode->key = key;
    pNode->parent = NULL;
    pNode->left = NULL;
    pNode->right = NULL;
}
    
bool CBinarySearchTree::insert(STSearchTreeNode* pNode)
{
    if (NULL == m_pRoot)
    {
        m_pRoot = pNode;
        ++m_nLen;
        return true;
    }

    STSearchTreeNode *pTmp = m_pRoot;
    STSearchTreeNode* pParent = NULL;
    while (pTmp)
    {
        pParent = pTmp;
        if (pNode->key < pTmp->key)
        {
            pTmp = pTmp->left;
        }
        else
        {
            pTmp = pTmp->left;
        }
    }
    if (NULL != pParent)
    {
        if (pParent->key < pNode->key)
        {
            pParent->left = pNode;
        }
        else
        {
            pParent->right = pNode;
        }
        pNode->parent = pParent;
    }
    
    return true;
}

void CBinarySearchTree::inorder_tree_walk(STSearchTreeNode* pNode, TKey* pOut, int& nPos)
{
    if (NULL == pNode)
    {
        return;
    }

    pOut[nPos++] = pNode->key;
    inorder_tree_walk(pNode->left, pOut, nPos);
    inorder_tree_walk(pNode->right, pOut, nPos);
}

void CBinarySearchTree::transplant(STSearchTreeNode* u, STSearchTreeNode* v)
{
    if (u->parent == NULL)
    {
        m_pRoot = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }

    if (v != NULL)
    {
        v->parent = u->parent;
    }
}