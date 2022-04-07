#pragma once
#include <mpl/typelist/TypeList.h>
#include <mpl/typelist/Element.h>
#include <mpl/typelist/FindBest.h>
#include <mpl/typelist/Length.h>
#include <mpl/typelist/Remove.h>
#include <mpl/typelist/Append.h>
#include <mpl/typelist/Replace.h>
#include <mpl/Int2Type.h>

namespace mpl
{
    namespace typelist
    {
/*
algoritmo:

*/


        //Sort result:
        // Indexes: TypeList of Int2Type<n> indicating new index for original element
        // Sorted: Sorted TypeList
        template <class L,class I> struct SortResult
        {
            typedef L Sorted;
            typedef I Indexes;
        };
        namespace _private
        {
            /**
            * Sort TypeList according to Condition (as > operator)
            * @todo todavia fulero, uso algoritmo burbuja
            **/
            template <class TList,template <class> class Condition,class IndexList,class OriginalIndexes> struct _Sort
            {
                private:
                    enum{best = FindBest<TList,Condition>::result};
                    typedef typename Element<TList,best,true>::Result Best;
                    enum{ origindex = Element<OriginalIndexes,best,true>::Result::value};
                    typedef typename Remove< OriginalIndexes,best>::Result _NewOriginalIndexes;
                    typedef typename Replace< IndexList,origindex,Int2Type< Length< TList>::result-1 > >::Result NewIndexList;
                    typedef typename _Sort< typename Remove< TList,best>::Result,Condition,NewIndexList,_NewOriginalIndexes>::Result AuxResult;
                    typedef typename AuxResult::Sorted AuxSorted;
                    typedef typename AuxResult::Indexes AuxIndexes;



                public:
                    typedef SortResult< typename Append< AuxSorted, Best>::Result,AuxIndexes> Result;
                 //  typedef SortResult< typename Append< Aux, Best>::Result,_NewOriginalIndexes > Result; el problema esta aqui, tengo que coger los indices del result
            };
            template <template <class> class Condition,class IndexList,class OriginalIndexes> struct _Sort<NullType,Condition,IndexList,OriginalIndexes>
            {
                typedef SortResult<NullType,IndexList> Result;
                //typedef SortResult<NullType,OriginalIndexes> Result;
            };

            //!Create TypeList of size "size" with type secuence of Int2Type
            template <int totalSize,int size> struct _CreateIndexes
            {
                typedef TypeList<Int2Type<totalSize - size>,typename _CreateIndexes< totalSize,size-1>::Result> Result;
            };
            template <int totalSize > struct _CreateIndexes<totalSize,1>
            {
                typedef TYPELIST( Int2Type<totalSize-1>) Result;
            };


        }
        template <class TList,template <class> class Condition> struct Sort
        {
            //NO ENTIENDO POR QUË NO ME DEJA HACIENDOLO PRIVATE
           // template <class,template <class> class,class> friend class _private::_Sort;
           // private:
                typedef typename Create< Length<TList>::result,NullType>::Result _IndexList;
                typedef typename _private::_CreateIndexes< Length<TList>::result,Length<TList>::result>::Result _IndexSequence;

           // necesito crear un TypeList a partir del tamaño y el tipo
            typedef typename _private::_Sort< TList, Condition,_IndexList,_IndexSequence>::Result Result;
        };
    }
}
