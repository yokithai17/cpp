#include <memory>
#include <list>

template <typename T, typename Allocator = std::allocator<T>>
class list {
private:
    struct Node;

    using node_allocator = typename 
    std::allocator_traits<Allocator>::template
    rebind_alloc<Node>;
    
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

    using iterator = based_iterator<false>;
    using const_iterator = based_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit list() : dummy_head(BaseNode()), sz(0), alloc() {
        link_nodes(&head, &head);
    }

    explicit list(size_type size): list() {
        spawn(size);
    }

    explicit list(size_type size, const_reference data) : list() {
        spawn_with_example(size, data);
    }

    explicit list(const allocator_type& alloc) : dummy_head(BaseNode()), sz(0), alloc(alloc) {
        link_nodes(&head, &head);
    }

    explicit list(size_type size, const allocator_type& alloc) : list(alloc) {
        spawn(size);
    }

    explicit list(size_type size, const_reference data
        , const allocator_type& alloc) : list(alloc) {
        spawn_with_example(size, data);
    }

    ~list() {
        this->clear();
    }

    void clear() {
        BaseNode* cur = dummy_head.next;
        while (cur != &dummy_head) {
            cur = cur->next;
            std::allocator_traits<node_allocator>::destroy(alloc, static_cast<Node*>(cur->prev));
            std::allocator_traits<node_allocator>::deallocate(alloc, static_cast<Node*>(cur->prev), 1);
        }
        sz = 0;
    }

    void push_back(const_reference data) {
        try {
            Node* temp = std::allocator_traits<node_allocator>::allocate(alloc, 1);
            std::allocator_traits<node_allocator>::construct(alloc, temp, data);
            link_nodes(dummy_head.prev, temp);
            link_nodes(temp, &dummy_head);
            ++sz;
        } catch  (...) {
            throw;
        }
    }

    void push_front(const_reference data) {
        try {
            Node* temp = std::allocator_traits<node_allocator>::allocate(alloc, 1);
            std::allocator_traits<node_allocator>::construct(alloc, temp, data);
            link_nodes(temp, dummy_head.next);
            link_nodes(&dummy_head, temp);
            ++sz;
        } catch  (...) {
            throw;
        }
    }

    void pop_front() {
        BaseNode* next = dummy_head.next->next;
        NodeTraits::destroy(allocator, static_cast<Node*>(dummy_head.next));
        NodeTraits::deallocate(allocator, static_cast<Node*>(dummy_head.next), 1);
        link_nodes(&dummy_head, next);
        --sz;
    }

    void pop_back() {
        BaseNode* prev = dummy_head.prev->prev;
        NodeTraits::destroy(allocator, static_cast<Node*>(dummy_head.prev));
        NodeTraits::deallocate(allocator, static_cast<Node*>(dummy_head.prev), 1);
        link_nodes(prev, &dummy_head);
        --sz;
    }

    list& operator=(const list& other) {
        if (&other == this) {
            return *this;
        }

        BaseNode* ne = dummy_head.next;
        BaseNode* prev = dummy_head.prev;
        size_type saved = sz;
        link_nodes(&dummy_head, &dummy_head);
        
        try {
            using NodeTraits = std::allocator_traits<node_allocator>;
            
            if (NodeTraits::propagate_on_container_copy_assignment::value) {
                node_allocator buf = other.alloc;
                spawn_from_other(other, buf);
            } else {
                spawn_from_other(other, alloc);
            }

            std::swap(ne, dummy_head.next);
            std::swap(prev, dummy_head.prev);

            clear();

            dummy_head.next = ne;
            dummy_head.prev = prev;
            sz = other.sz;
            if (NodeTraits::propagate_on_container_copy_assignment::value) {
                alloc = other.alloc;
            }
        } catch (...) {
            dummy_head.next = ne;
            dummy_head.prev = prev;
            sz = saved;
            throw;
        }

        return *this;
    }


private:
    struct BaseNode {
        BaseNode* prev = nullptr;
        BaseNode* next = nullptr;
    };

    struct Node : BaseNode {
        value_type data;

        explicit Node(const_reference data) : data(data) {}

        explicit Node() : data() {}
    };

    template <bool IsConst>
    class based_iterator {
    public:
        using value_type = typename std::conditional<IsConst, const T, T>::type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using reference = value_type&;
        using pointer = value_type*;

        explicit based_iterator(const based_iterator&) = default;

        based_iterator& operator=(const based_iterator&) = default;

        bool operator==(const based_iterator& other) const {
            return node == other.node;
        }

        bool operator!=(const based_iterator& other) const {
            return node != other.node;
        }

        reference operator*() {
            return static_cast<Node*>(node)->data;
        }

        const T& operator*() const {
            return static_cast<Node*>(node)->data;
        }

        pointer operator->() {
            return &(static_cast<Node*>(node)->data);
        }

        const T* operator->() const {
            return &(static_cast<Node*>(node)->data);
        }
    
    private: 
        BaseNode* node;

        explicit based_iterator(BaseNode* node) : node(node) {}
    };

    void link_nodes(BaseNode* left, BaseNode* right) {
        left->next = right;
        right->prev = left;
    }

    void spawn(size_type count) {
        if (count == 0) {
            return;
        }

        BaseNode* last = &head;
        Node* node_p = nullptr;
        size_type start = 0;
        try {
            for (; start < count; ++start) {
                Node* cur = std::allocator_traits<node_allocator>::allocate(alloc, 1);
                std::allocator_traits<node_allocator>::construct(alloc, cur);
                link_nodes(last, cur);
                last = cur;
            }
        } catch (...) {
            for (size_type j = 0; j < start) {
                Node* prev = last->prev;
                std::allocator_traits<node_allocator>::destroy(alloc, last);
                std::allocator_traits<node_allocator>::deallocate(alloc, last);
                last = static_cast<Node*>(prev);
            }
            throw;
        }

        this->sz = count;
    }

    void spawn_with_example(size_type count, const_reference data) {
        if (count == 0) {
            return;
        }

        BaseNode* last = &head;
        Node* node_p = nullptr;
        size_type start = 0;
        try {
            for (; start < count; ++start) {
                Node* cur = std::allocator_traits<node_allocator>::allocate(alloc, 1);
                std::allocator_traits<node_allocator>::construct(alloc, cur, data);
                link_nodes(last, cur);
                last = cur;
            }
        } catch (...) {
            for (size_type j = 0; j < start) {
                Node* prev = last->prev;
                std::allocator_traits<node_allocator>::destroy(alloc, last);
                std::allocator_traits<node_allocator>::deallocate(alloc, last);
                last = static_cast<Node*>(prev);
            }
            throw;
        }

        this->sz = count;
    }

    void spawn_from_other(const list& list, node_allocator&) {

    }
    
    BaseNode dummy_head;
    size_type sz;
    [[ no_unique_address ]] node_allocator alloc;
};