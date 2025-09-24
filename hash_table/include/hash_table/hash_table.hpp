#include <vector>
#include <list>
#include <mutex>


class HashTable
{
  public:
    HashTable(const unsigned size);

    void insert(int orderID, int productID); // Work as create and as modify.
    int get(int orderID); // -1 if orderID not present.
    void erase(int orderID);

  private:
    int hash_function(int key) const;
    void rehash_if_overload();
    
    unsigned m_size; // Ideally a power of 2.
    std::vector<std::list<std::pair<int, int>>> m_table;
    const unsigned w{32}; // Word size in bits (constexpr only if static).
    unsigned p; // log2(m_size).
    unsigned m_count{0}; // To compute the load factor alpha.
    std::mutex m_semaphore;
};

HashTable::HashTable(const unsigned size)
: m_size{size}, m_table(m_size, std::list<std::pair<int,int>>{})
{
    // Use only m_size%2==0 so only values in [0, m_size-1] and optimal use of 
    //  log2(m_size)-1 bits.
    unsigned N = m_size - 1;
    for (p = 0u; N > 0; N >>= 1, p++);
}

int HashTable::hash_function(int key) const
{
    // The Knuth golden ration constant: floor(golden_ration*(2^32)).
    // Experiments shows is good for m_size up to 100k, even if not prime.
    constexpr uint32_t A = 2'654'435'761U;

    // The shift takes the upper log2(m_size-1) bits, that is the number of bits
    //  necessary to represent the values in [0, m_size-1]. Take the upper and 
    //  not the lower bits, because the upper ones change slower so more 
    //  variability and less conflicts.
    // Equivalent to (key*A)%m_size - because m_size%2==0 - but more efficient. 
    //  If m_size%2!=0 you can't use it because of the value of 'p' and you 
    //  should use only (key*A)%m_size.
    return (key * A) >> (w - p);
}

void HashTable::rehash_if_overload()
{
    // Check the Alpha Load Factor.
    if ((m_count+0.0) / m_size > 0.75)
    {
        // Overload, so double m_size...
        
        // ... and rehash.
    }
}

void HashTable::insert(int orderID, int productID)
{
    // Less overhead than unique_lock, but you can't control it, so no use in 
    //  condition variable.
    std::lock_guard<std::mutex> lock(m_semaphore);

    auto& bucket = m_table[hash_function(orderID)];
    for (auto& entry : bucket)
    {
        if (entry.first == orderID)
        {
            entry.second = productID;
            return;
        }
    }
    // Here if new orderID.
    bucket.push_front({orderID, productID});
    m_count += 1;

    rehash_if_overload();
}

int HashTable::get(int orderID)
{
    // Shared lock for read-only, but mutex must be shared_mutex.
    std::lock_guard<std::mutex> lock(m_semaphore);

    auto& bucket = m_table[hash_function(orderID)];
    for (auto& entry : bucket)
    {
        if (entry.first == orderID)
        {
            return entry.second;
        }
    }
    return -1;
}

void HashTable::erase(int orderID)
{
    std::lock_guard<std::mutex> lock(m_semaphore);
    
    auto& bucket = m_table[hash_function(orderID)];
    auto prev_size = bucket.size();
    // More idiomatic then a for-loop plus erase().
    bucket.remove_if([orderID](const auto& entry){
      return entry.first == orderID;
    });

    m_count -= (prev_size - bucket.size());
}
