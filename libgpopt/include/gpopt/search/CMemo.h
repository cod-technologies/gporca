//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2009 - 2011 EMC Corp.
//
//	@filename:
//		CMemo.h
//
//	@doc:
//		Memo lookup table for dynamic programming
//---------------------------------------------------------------------------
#ifndef GPOPT_CMemo_H
#define GPOPT_CMemo_H

#include "gpos/base.h"
#include "gpos/common/CRefCount.h"
#include "gpos/common/CSyncHashtable.h"
#include "gpos/common/CSyncList.h"
#include "gpos/sync/CAtomicCounter.h"

#include "gpopt/spinlock.h"
#include "gpopt/search/CGroupExpression.h"

namespace gpopt
{
	class CGroup;
	class CDrvdProp;
	class CDrvdPropCtxtPlan;
	class CMemoProxy;
	class COptimizationContext;

	// memo tree map definition
	typedef CTreeMap<CCostContext, CExpression, CDrvdPropCtxtPlan, CCostContext::UlHash, CCostContext::FEqual> MemoTreeMap;

	using namespace gpos;
	
	//---------------------------------------------------------------------------
	//	@class:
	//		CMemo
	//
	//	@doc:
	//		Dynamic programming table
	//
	//---------------------------------------------------------------------------
	class CMemo
	{
		private:
		
			// definition of hash table key accessor
			typedef
					CSyncHashtableAccessByKey<
						CGroupExpression, // entry
						CGroupExpression, // search key
						CSpinlockMemo> ShtAcc;

			// definition of hash table iterator
			typedef
					CSyncHashtableIter<
						CGroupExpression, // entry
						CGroupExpression, // search key
						CSpinlockMemo> ShtIter;

			// definition of hash table iterator accessor
			typedef
					CSyncHashtableAccessByIter<
						CGroupExpression, // entry
						CGroupExpression, // search key
						CSpinlockMemo> ShtAccIter;

			// memory pool
			IMemoryPool *m_pmp;
		
			// id counter for groups
			CAtomicULONG m_aul;

			// root group
			CGroup *m_pgroupRoot;

			// number of groups
			volatile ULONG_PTR m_ulpGrps;

			// tree map of member group expressions
			MemoTreeMap *m_pmemotmap;

			// list of groups
			CSyncList<CGroup> m_listGroups;

			// hashtable of all group expressions
			CSyncHashtable<
				CGroupExpression, // entry
				CGroupExpression, // search key
				CSpinlockMemo> m_sht;

			// add new group
			void Add(CGroup *pgroup, CExpression *pexprOrigin);

			// rehash all group expressions after group merge - not thread-safe
			BOOL FRehash();

			// helper for inserting group expression in target group
			CGroup *PgroupInsert(CGroup *pgroupTarget, CGroupExpression *pgexpr, CExpression *pexprOrigin, BOOL fNewGroup);

			// helper to check if a new group needs to be created
			BOOL FNewGroup(CGroup **ppgroupTarget, CGroupExpression *pgexpr, BOOL fScalar);

			// private copy ctor
			CMemo(const CMemo &);
						
		public:
		
			// ctor
			explicit
			CMemo(IMemoryPool *pmp);
			
			// dtor
			~CMemo();
			
			// return root group
			CGroup *PgroupRoot() const
			{
				return m_pgroupRoot;
			}

			// return number of groups
			ULONG_PTR UlpGroups() const
			{
				return m_ulpGrps;
			}

			// return total number of group expressions
			ULONG UlGrpExprs();

			// return number of duplicate groups
			ULONG UlDuplicateGroups();

			// mark groups as duplicates
			void MarkDuplicates(CGroup *pgroupFst, CGroup *pgroupSnd);

			// return tree map
			MemoTreeMap *Pmemotmap() const
			{
				return m_pmemotmap;
			}

			// set root group
			void SetRoot(CGroup *pgroup);

			// insert group expression into hash table
			CGroup *PgroupInsert(CGroup *pgroupTarget, CExpression *pexprOrigin, CGroupExpression *pgexpr);

			// extract a plan that delivers the given required properties
			CExpression *PexprExtractPlan(IMemoryPool *pmp, CGroup *pgroupRoot, CReqdPropPlan *prppInput, ULONG ulSearchStages);

			// merge duplicate groups
			void GroupMerge();

			// reset states of all memo groups
			void ResetGroupStates();

			// reset statistics of memo groups
			void ResetStats();

			// print driver
			IOstream &OsPrint(IOstream &os);

			// derive stats when no stats not present for the group
			void DeriveStatsIfAbsent(IMemoryPool *pmp);

			// build tree map
			void BuildTreeMap(COptimizationContext *poc);

			// reset tree map
			void ResetTreeMap();

			// print memo to output logger
			void Trace();

#ifdef GPOS_DEBUG
			// get group by id
			CGroup *Pgroup(ULONG ulId);
#endif // GPOS_DEBUG

	}; // class CMemo
	
}

#endif // !GPOPT_CMemo_H

// EOF
