#pragma once
#include "Reflection.h"

namespace SC
{

namespace Reflection
{

template <int TOTAL_ATOMS>
struct FlatSchema
{
    AtomsArray<AtomProperties, TOTAL_ATOMS> atoms;
    AtomsArray<const char*, TOTAL_ATOMS>    names;
};

struct FlatAtomLink
{
    typedef typename Atom::AtomsPushFunc AtomsPushFunc;

    AtomsPushFunc pushAtomsTo;
    int           flatternedIndex;
    int           numberOfAtoms;

    constexpr FlatAtomLink() : pushAtomsTo(nullptr), flatternedIndex(0), numberOfAtoms(0) {}

    template <int MAX_ATOMS>
    constexpr auto getAtoms() const
    {
        return GetAtomsFromFunc<MAX_ATOMS>(pushAtomsTo);
    }
};

template <int MAX_ATOMS = 20>
struct FlatSchemaCompiler
{
    template <int MAX_POSSIBLE_LINKS>
    static constexpr int countUniqueLinks(const AtomsArray<Atom, MAX_ATOMS>& rootAtom)
    {
        AtomsArray<AtomsArray<Atom, MAX_ATOMS>, MAX_POSSIBLE_LINKS> atomsQueue;
        atomsQueue.values[atomsQueue.size++] = rootAtom;
        AtomsArray<typename Atom::AtomsPushFunc, MAX_POSSIBLE_LINKS> alreadyVisitedLinks;

        int numLinks = 1;

        while (atomsQueue.size > 0)
        {
            atomsQueue.size--;
            const auto atomsChildren = atomsQueue.values[atomsQueue.size]; // MUST copy as we're modifying queue
            for (int idx = 0; idx < atomsChildren.values[0].properties.numSubAtoms; ++idx)
            {
                const auto& atom = atomsChildren.values[idx + 1];

                bool alreadyVisitedLink = false;
                for (int searchIDX = 0; searchIDX < alreadyVisitedLinks.size; ++searchIDX)
                {
                    if (alreadyVisitedLinks.values[searchIDX] == atom.pushAtomsTo)
                    {
                        alreadyVisitedLink = true;
                        break;
                    }
                }
                if (not alreadyVisitedLink)
                {
                    alreadyVisitedLinks.values[alreadyVisitedLinks.size++] = atom.pushAtomsTo;
                    const auto linkAtoms                                   = atom.template getAtoms<MAX_ATOMS>();
                    if (atom.properties.type == AtomType::TypeInvalid)
                    {
                        return -1; // Missing descriptor for type
                    }
                    else if (linkAtoms.size > 0)
                    {
                        numLinks++;
                        atomsQueue.values[atomsQueue.size++] = linkAtoms;
                    }
                    else if (atom.properties.type == AtomType::TypeStruct)
                    {
                        return -2; // Somebody created a struct with empty list of atoms
                    }
                }
            }
        }
        return numLinks;
    }

    template <int UNIQUE_LINKS_NUMBER, int MAX_POSSIBLE_LINKS>
    static constexpr auto findAllLinks(const AtomsArray<Atom, MAX_ATOMS>& inputAtoms,
                                       typename Atom::AtomsPushFunc       rootPushAtomsTo)
    {
        AtomsArray<FlatAtomLink, UNIQUE_LINKS_NUMBER>               links;
        AtomsArray<AtomsArray<Atom, MAX_ATOMS>, MAX_POSSIBLE_LINKS> atomsQueue;
        atomsQueue.values[atomsQueue.size++]     = inputAtoms;
        links.values[links.size].pushAtomsTo     = rootPushAtomsTo;
        links.values[links.size].flatternedIndex = 0;
        links.values[links.size].numberOfAtoms   = inputAtoms.size;
        links.size++;
        while (atomsQueue.size > 0)
        {
            atomsQueue.size--;
            const auto rootAtom = atomsQueue.values[atomsQueue.size]; // MUST be copy (we modify atomsQueue)
            for (int atomIdx = 0; atomIdx < rootAtom.values[0].properties.numSubAtoms; ++atomIdx)
            {
                const auto& atom        = rootAtom.values[atomIdx + 1];
                auto        pushAtomsTo = atom.pushAtomsTo;
                bool        foundLink   = false;
                for (int searchIDX = 0; searchIDX < links.size; ++searchIDX)
                {
                    if (links.values[searchIDX].pushAtomsTo == pushAtomsTo)
                    {
                        foundLink = true;
                        break;
                    }
                }
                if (not foundLink)
                {
                    auto linkAtoms = atom.template getAtoms<MAX_ATOMS>();
                    if (linkAtoms.size > 0)
                    {
                        atomsQueue.values[atomsQueue.size++] = linkAtoms;
                        int prevMembers                      = 0;
                        if (links.size > 0)
                        {
                            const auto& prev = links.values[links.size - 1];
                            prevMembers      = prev.flatternedIndex + prev.numberOfAtoms;
                        }

                        links.values[links.size].pushAtomsTo     = pushAtomsTo;
                        links.values[links.size].flatternedIndex = prevMembers;
                        links.values[links.size].numberOfAtoms   = linkAtoms.size;
                        links.size++;
                    }
                }
            }
        }
        return links;
    }

    template <int TOTAL_ATOMS, int MAX_LINKS_NUMBER>
    static constexpr void mergeLinksFlat(const AtomsArray<FlatAtomLink, MAX_LINKS_NUMBER>& links,
                                         AtomsArray<AtomProperties, TOTAL_ATOMS>&          mergedAtoms,
                                         AtomsArray<const char*, TOTAL_ATOMS>*             mergedNames)
    {
        for (int linkIndex = 0; linkIndex < links.size; ++linkIndex)
        {
            auto linkAtoms                         = links.values[linkIndex].template getAtoms<MAX_ATOMS>();
            mergedAtoms.values[mergedAtoms.size++] = linkAtoms.values[0].properties;
            if (mergedNames)
            {
                mergedNames->values[mergedNames->size++] = linkAtoms.values[0].name;
            }
            for (int atomIndex = 0; atomIndex < linkAtoms.values[0].properties.numSubAtoms; ++atomIndex)
            {
                const auto& field                    = linkAtoms.values[1 + atomIndex];
                mergedAtoms.values[mergedAtoms.size] = field.properties;
                if (mergedNames)
                {
                    mergedNames->values[mergedNames->size++] = field.name;
                }
                for (int findIdx = 0; findIdx < links.size; ++findIdx)
                {
                    if (links.values[findIdx].pushAtomsTo == field.pushAtomsTo)
                    {
                        mergedAtoms.values[mergedAtoms.size].setLinkIndex(links.values[findIdx].flatternedIndex);
                        break;
                    }
                }
                mergedAtoms.size++;
            }
        }
    }

    // You can customize MAX_ATOMS to match the max number of atoms (+1) of any descriptor that will be linked
    // You can customize MAX_POSSIBLE_LINKS for the max numer of unique types in the system
    // This is just consuming compile time memory, so it doesn't matter putting any value as long as your compiler
    // is able to handle it without running out of heap space :)
    template <typename T, int MAX_POSSIBLE_LINKS = 500>
    static constexpr auto compile()
    {
        constexpr auto linkAtoms = GetAtomsFor<T, MAX_ATOMS>();
        static_assert(linkAtoms.size > 0, "Missing Descriptor for root class");
        constexpr auto UNIQUE_LINKS_NUMBER = countUniqueLinks<MAX_POSSIBLE_LINKS>(linkAtoms);
        static_assert(UNIQUE_LINKS_NUMBER >= 0, "Missing Descriptor for a class reachable by root class");
        constexpr auto links =
            findAllLinks<UNIQUE_LINKS_NUMBER, MAX_POSSIBLE_LINKS>(linkAtoms, &AtomsFor<T>::pushAtomsTo);
        constexpr auto TOTAL_ATOMS =
            links.values[links.size - 1].flatternedIndex + links.values[links.size - 1].numberOfAtoms;
        FlatSchema<TOTAL_ATOMS> result;
        mergeLinksFlat(links, result.atoms, &result.names);
        return result;
    }
};
} // namespace Reflection
} // namespace SC
