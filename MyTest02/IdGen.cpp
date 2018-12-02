#ifndef _ID_GEN_
#define _ID_GEN_

#include <set>

class IDGen
{
public:

    static std::set<unsigned long> s_usedID;

    static unsigned long generateID() // Generate a valid ID
    {
        static unsigned long id = 0;

        while (IDGen::isIDUsed(id)) // If all ID are taken, create an infinite loop!
            ++id;

        return id;
    }

    static void addID(unsigned long id) // Add a given ID to the set of used ID
    {
        s_usedID.insert(id);
    }

    static void removeID(unsigned long id) // Remove a given ID from the set of used ID
    {
        s_usedID.erase(id);
    }

    static bool isIDUsed(unsigned long id)
    {
        return s_usedID.count(id) == 1 ? true : false;
    }

    explicit IDGen() : m_id(IDGen::generateID())
    {
        IDGen::addID(m_id); // ID is now taken
    }

    virtual ~IDGen()
    {
        IDGen::removeID(m_id); // Free the ID
    }

private:

    unsigned long m_id;

};

#endif