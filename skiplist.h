#include <iostream>
#include <cmath>
#include <mutex>
#include <fstream>
#include <cstring>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;
std::string delimiter = ":";

//节点类
template <typename K, typename V>
class Node
{
public:
    Node(){};
    Node(K k, V v, int);
    ~Node();
    K get_key() const;
    V get_value() const;
    void set_value(V);

    //存放不同层中下一个节点的指针的线性表
    Node<K, V> **forward;
    int node_level;

private:
    K key;
    V value;
};

template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level)
{
    this->key = k;
    this->value = v;
    this->node_level = level;

    //从第0层开始
    this->forward = new Node<K, V> *[level + 1];
    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
}

template <typename K, typename V>
Node<K, V>::~Node()
{
    delete[] forward;
}

template <typename K, typename V>
K Node<K, V>::get_key() const
{
    return key;
}

template <typename K, typename V>
V Node<K, V>::get_value() const
{
    return value;
}

template <typename K, typename V>
void Node<K, V>::set_value(V value)
{
    this->value = value;
}

// skiplist类
template <typename K, typename V>
class Skiplist
{

public:
    Skiplist(int);
    ~Skiplist();
    int get_random_level();
    Node<K, V> *create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string &str, std::string *key, std::string *value);
    bool is_valid_string(const std::string &str);

private:
    int _max_level;

    // Current level of skiplist
    int _skiplist_level;

    // pointer to header node
    Node<K, V> *_header;

    // file operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // skiplist current element count
    int _element_count;
};

template <typename K, typename V>
Node<K, V> *Skiplist<K, V>::create_node(const K k, const V v, int level)
{
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// Insert given key and value in skip list
// return 1 means element exists
// return 0 means insert successfully
/*
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/

template <typename K, typename V>
int Skiplist<K, V>::insert_element(const K key, const V value)
{
    mtx.lock();
    Node<K, V> *current = this->_header;
    // create update array and initialize it
    // update is array which put node that the node
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    // start from highest level of skiplist
    for (int i = _skiplist_level; i >= 0; i--)
    {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }
    // reach level 0 and forward pointer to right node, which is desired to insert key
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if (current != NULL && current->get_key() == key)
    {
        std::cout << "key:" << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // if current is NULL that means we have to reach the end of the level
    // if current's key is not equal to key that means we have to insert node between update[0] and current node
    if (current == NULL || current->get_key() != key)
    {
        // generate a random level for node
        int random_level = get_random_level();

        // if random level is greater than skiplist's current level , initialize update value with pointer to header
        if (random_level > _skiplist_level)
        {
            for (int i = _skiplist_level + 1; i < random_level + 1; i++)
            {
                update[i] = _header;
            }
            _skiplist_level = random_level;
        }

        // create new node with random level generated
        Node<K, V> *inserted_node = create_node(key, value, random_level);

        // inseret node
        for (int i = 0; i <= random_level; i++)
        {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count++;
    }
    mtx.unlock();
    return 0;
}

// display skiplist
template <typename K, typename V>
void Skiplist<K, V>::display_list()
{
    std::cout << "\n******Skiplist******"
              << "\n";
    for (int i = 0; i <= _skiplist_level; i++)
    {
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL)
        {
            std::cout << node->get_key() << ": " << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// Dump data in memory to file
template <typename K, typename V>
void Skiplist<K, V>::dump_file()
{
    std::cout << "dump file------------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];
    while (node != NULL)
    {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << "\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
}

// load data from disk
template <typename K, typename V>
void Skiplist<K, V>::load_file()
{

    _file_reader.open(STORE_FILE);
    std::cout << "load_file--------------------" << std::endl;
    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty())
        {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << value << std::endl;
    }
    _file_reader.close();
}

// get current Skiplist size
template <typename K, typename V>
int Skiplist<K, V>::size()
{
    return _element_count;
}

template <typename K, typename V>
void Skiplist<K, V>::get_key_value_from_string(const std::string &str, std::string *key, std::string *value)
{
    if (!is_valid_string(str))
    {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template <typename K, typename V>
bool Skiplist<K, V>::is_valid_string(const std::string &str)
{

    if (str.empty())
        return false;
    if (str.find(delimiter) == std::string::npos)
        return false;

    return true;
}

// Delet element from skip list
template <typename K, typename V>
void Skiplist<K, V>::delete_element(K key)
{

    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    // start from highest level of skip list
    for (int i = _skiplist_level; i >= 0; i--)
    {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    if (current != NULL && current->get_key() == key)
    {
        // start for lowest level and delete the current node of each level
        for (int i = 0; i <= _skiplist_level; i++)
        {
            // if at level i, next node is no target node, breake the loop
            if (update[i]->forward[i] != current)
                break;

            update[i]->forward[i] = current->forward[i];
        }
        delete current;
        // remove levels which has no elements
        while (_skiplist_level > 0 && _header->forward[_skiplist_level] == 0)
        {
            _skiplist_level--;
        }
        std::cout << "Successfully delete key " << key << std::endl;
        _element_count--;
    }
    mtx.unlock();
    return;
}

// Search for element in skip list
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template <typename K, typename V>
bool Skiplist<K, V>::search_element(K key)
{
    std::cout << "Search element---------------------------" << std::endl;
    Node<K, V> *current = _header;

    // Search from the highest level of skiplist
    for (int i = _skiplist_level; i >= 0; i--)
    {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
    }
    // Reach level 0 and advance pointer to right node
    current = current->forward[0];

    // if the key match, we find it
    if (current && current->get_key() == key)
    {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// Construct skiplist
template <typename K, typename V>
Skiplist<K, V>::Skiplist(int max_level)
{
    this->_max_level = max_level;
    this->_skiplist_level = 0;
    this->_element_count = 0;
    // Create header node and initialize key and value
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, max_level);
};

template <typename K, typename V>
Skiplist<K, V>::~Skiplist()
{
    if (_file_writer.is_open())
    {
        _file_writer.close();
    }
    if (_file_reader.is_open())
    {
        _file_reader.close();
    }
    delete _header;
}

template <typename K, typename V>
int Skiplist<K, V>::get_random_level()
{
    int k = 1;
    while (rand() % 2)
    {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
}