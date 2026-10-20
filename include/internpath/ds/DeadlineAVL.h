#pragma once

#include <cstdint>
#include <stdexcept>

#include "internpath/ds/DynamicArray.h"

namespace internpath
{
    struct DeadlineKey
    {
        std::int32_t deadline = 0;
        std::uint32_t internshipId = 0;
    };

    // MEMBER B | TODO B06-B09 | AVL indexed by (deadline, internshipId).
    class DeadlineAVL
    {
    private:
        struct Node
        {
            DeadlineKey key;
            Node* left = nullptr;
            Node* right = nullptr;
            int height = 1;
        };

        Node* root_ = nullptr;
        std::size_t size_ = 0;

        static int compareKey(const DeadlineKey& a, const DeadlineKey& b)
        {
            if (a.deadline < b.deadline)
            {
                return -1;
            }

            if (a.deadline > b.deadline)
            {
                return 1;
            }

            if (a.internshipId < b.internshipId)
            {
                return -1;
            }

            if (a.internshipId > b.internshipId)
            {
                return 1;
            }

            return 0;
        }

        static int heightOf(Node* node)
        {
            return node == nullptr ? 0 : node->height;
        }

        static int balanceFactor(Node* node)
        {
            return node == nullptr ? 0 : heightOf(node->left) - heightOf(node->right);
        }

        static void updateHeight(Node* node)
        {
            if (node != nullptr)
            {
                int hl = heightOf(node->left);
                int hr = heightOf(node->right);
                node->height = 1 + (hl > hr ? hl : hr);
            }
        }

        static Node* rotateRight(Node* y)
        {
            Node* x = y->left;
            Node* t2 = x->right;

            x->right = y;
            y->left = t2;

            updateHeight(y);
            updateHeight(x);

            return x;
        }

        static Node* rotateLeft(Node* x)
        {
            Node* y = x->right;
            Node* t2 = y->left;

            y->left = x;
            x->right = t2;

            updateHeight(x);
            updateHeight(y);

            return y;
        }

        static Node* rebalance(Node* node)
        {
            if (node == nullptr)
            {
                return nullptr;
            }

            updateHeight(node);
            int bf = balanceFactor(node);

            // Left heavy
            if (bf > 1)
            {
                // Left-Right case
                if (balanceFactor(node->left) < 0)
                {
                    node->left = rotateLeft(node->left);
                }

                return rotateRight(node);
            }

            // Right heavy
            if (bf < -1)
            {
                // Right-Left case
                if (balanceFactor(node->right) > 0)
                {
                    node->right = rotateRight(node->right);
                }

                return rotateLeft(node);
            }

            return node;
        }

        static Node* insertNode(Node* node, const DeadlineKey& key, bool& inserted)
        {
            if (node == nullptr)
            {
                inserted = true;
                return new Node{key, nullptr, nullptr, 1};
            }

            int cmp = compareKey(key, node->key);

            if (cmp < 0)
            {
                node->left = insertNode(node->left, key, inserted);
            }
            else if (cmp > 0)
            {
                node->right = insertNode(node->right, key, inserted);
            }
            else
            {
                inserted = false;
                return node;
            }

            return rebalance(node);
        }

        static Node* findMin(Node* node)
        {
            Node* current = node;

            while (current != nullptr && current->left != nullptr)
            {
                current = current->left;
            }

            return current;
        }

        static Node* eraseNode(Node* node, const DeadlineKey& key, bool& erased)
        {
            if (node == nullptr)
            {
                erased = false;
                return nullptr;
            }

            int cmp = compareKey(key, node->key);

            if (cmp < 0)
            {
                node->left = eraseNode(node->left, key, erased);
            }
            else if (cmp > 0)
            {
                node->right = eraseNode(node->right, key, erased);
            }
            else
            {
                erased = true;

                if (node->left == nullptr || node->right == nullptr)
                {
                    Node* temp = (node->left != nullptr) ? node->left : node->right;
                    delete node;
                    return temp;
                }
                else
                {
                    Node* successor = findMin(node->right);
                    node->key = successor->key;
                    node->right = eraseNode(node->right, successor->key, erased);
                }
            }

            return rebalance(node);
        }

        static void rangeQuery(Node* node, std::int32_t from, std::int32_t to, DynamicArray<std::uint32_t>& ids)
        {
            if (node == nullptr)
            {
                return;
            }

            // Pruning left subtree
            if (node->key.deadline >= from)
            {
                rangeQuery(node->left, from, to, ids);
            }

            // In range
            if (node->key.deadline >= from && node->key.deadline <= to)
            {
                ids.pushBack(node->key.internshipId);
            }

            // Pruning right subtree
            if (node->key.deadline <= to)
            {
                rangeQuery(node->right, from, to, ids);
            }
        }

        static bool validateSubtree(Node* node, const DeadlineKey* minKey, const DeadlineKey* maxKey)
        {
            if (node == nullptr)
            {
                return true;
            }

            if (minKey != nullptr && compareKey(node->key, *minKey) <= 0)
            {
                return false;
            }

            if (maxKey != nullptr && compareKey(node->key, *maxKey) >= 0)
            {
                return false;
            }

            int bf = balanceFactor(node);

            if (bf < -1 || bf > 1)
            {
                return false;
            }

            int hl = heightOf(node->left);
            int hr = heightOf(node->right);

            if (node->height != 1 + (hl > hr ? hl : hr))
            {
                return false;
            }

            return validateSubtree(node->left, minKey, &(node->key)) &&
                   validateSubtree(node->right, &(node->key), maxKey);
        }

        static void destroy(Node* node)
        {
            if (node == nullptr)
            {
                return;
            }

            destroy(node->left);
            destroy(node->right);
            delete node;
        }

    public:
        DeadlineAVL() = default;
        DeadlineAVL(const DeadlineAVL&) = delete;
        DeadlineAVL& operator=(const DeadlineAVL&) = delete;

        DeadlineAVL(DeadlineAVL&& other) noexcept
            : root_(other.root_), size_(other.size_)
        {
            other.root_ = nullptr;
            other.size_ = 0;
        }

        DeadlineAVL& operator=(DeadlineAVL&& other) noexcept
        {
            if (this != &other)
            {
                destroy(root_);
                root_ = other.root_;
                size_ = other.size_;

                other.root_ = nullptr;
                other.size_ = 0;
            }

            return *this;
        }

        ~DeadlineAVL()
        {
            destroy(root_);
        }

        std::size_t size() const
        {
            return size_;
        }

        bool empty() const
        {
            return size_ == 0;
        }

        void clear()
        {
            destroy(root_);
            root_ = nullptr;
            size_ = 0;
        }

        // TODO B06: Standard AVL insert with rotations
        bool insert(const DeadlineKey& key)
        {
            bool inserted = false;
            root_ = insertNode(root_, key, inserted);

            if (inserted)
            {
                size_++;
            }

            return inserted;
        }

        // TODO B07: BST removal with AVL rebalance
        bool erase(const DeadlineKey& key)
        {
            bool erased = false;
            root_ = eraseNode(root_, key, erased);

            if (erased)
            {
                size_--;
            }

            return erased;
        }

        // TODO B08: In-order range traversal with subtree pruning
        void range(std::int32_t from, std::int32_t to,
                   DynamicArray<std::uint32_t>& ids) const
        {
            rangeQuery(root_, from, to, ids);
        }

        // TODO B09: Validate BST ordering, node heights, and balance factor <= 1
        bool validate() const
        {
            return validateSubtree(root_, nullptr, nullptr);
        }
    };
}
