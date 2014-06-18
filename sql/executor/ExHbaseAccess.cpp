// **********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2013-2014 Hewlett-Packard Development Company, L.P.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// @@@ END COPYRIGHT @@@
// **********************************************************************

#include "Platform.h"

#include "ex_stdh.h"
#include "ComTdb.h"
#include "ex_tcb.h"
#include "ExHbaseAccess.h"
#include "ex_exe_stmt_globals.h"
#include "SQLTypeDefs.h"

#include "ExpHbaseInterface.h"
#include "HdfsLogger.h"

#include  "cli_stdh.h"
#include "exp_function.h"
#include "jni.h"

Int64 getTransactionIDFromContext()
{
  ExTransaction *ta = GetCliGlobals()->currContext()->getTransaction();
  Int64 currentXnId = (Int64) 0;
  Int64 transid = (Int64) 0;
  TmfPhandle_Struct currentTxHandle;

  short retcode = ta->getCurrentXnId((Int64 *)&currentXnId, (Int64 *)&transid, (short *)&currentTxHandle);

  return transid;
}

ex_tcb * ExHbaseAccessTdb::build(ex_globals * glob)
{
  ExExeStmtGlobals * exe_glob = glob->castToExExeStmtGlobals();
  
  ex_assert(exe_glob,"This operator cannot be in DP2");

  ExHbaseAccessTcb *tcb = NULL;

  if ((1) && //(sqHbaseTable()) &&
      ((accessType_ == UPDATE_) ||
       (accessType_ == MERGE_) ||
       (accessType_ == DELETE_)))
    {
      if (rowsetOper())
	tcb = new(exe_glob->getSpace()) 
	  ExHbaseAccessSQRowsetTcb(
				   *this,
				   exe_glob);
      else
	tcb = new(exe_glob->getSpace()) 
	  ExHbaseAccessUMDTcb(
				*this,
				exe_glob);
    }
  else if ((sqHbaseTable()) &&
	   (accessType_ == SELECT_) &&
	   (rowsetOper()))
    {
      tcb = new(exe_glob->getSpace()) 
	ExHbaseAccessSQRowsetTcb(
				 *this,
				 exe_glob);
    }
  else if (accessType_ == DELETE_)
    {
      /*
	tcb = new(exe_glob->getSpace()) 
	  ExHbaseAccessUMDHbaseTcb(
					*this,
					exe_glob);
      */
    }
  else if (accessType_ == SELECT_)
    {
      if (keyMDAMGen())
	{
	  // must be SQ Seabase table and no listOfScan/Get keys
	  if ((sqHbaseTable()) &&
	      (! listOfGetRows()) &&
	      (! listOfScanRows()))
	    {
	      tcb = new(exe_glob->getSpace()) 
		ExHbaseAccessMdamSelectTcb(
					   *this,
					   exe_glob);
	    }
	}
      else
	tcb = new(exe_glob->getSpace()) 
	  ExHbaseAccessSelectTcb(
				 *this,
				 exe_glob);
    }
  else if (accessType_ == COPROC_)
    {
      tcb = new(exe_glob->getSpace()) 
	ExHbaseAccessSelectTcb(
			       *this,
			       exe_glob);
    }
  else if ((accessType_ == INSERT_) ||
	   (accessType_ == UPSERT_) ||
	   (accessType_ == UPSERT_LOAD_))
    {
      if (sqHbaseTable())
	{
	  if ((vsbbInsert()) &&
	      (NOT hbaseSqlIUD()))
        if (this->getIsTrafodionLoadPrep())
              tcb = new(exe_glob->getSpace())
                        ExHbaseAccessBulkLoadPrepSQTcb(
                                                   *this,
                                                   exe_glob);
          else
	    tcb = new(exe_glob->getSpace()) 
	      ExHbaseAccessUpsertVsbbSQTcb(
					   *this,
					   exe_glob);
	  else
	    tcb = new(exe_glob->getSpace()) 
	      ExHbaseAccessInsertSQTcb(
				       *this,
				       exe_glob);
	}
      else
	{
	  if (rowwiseFormat())
	    tcb = new(exe_glob->getSpace()) 
	      ExHbaseAccessInsertRowwiseTcb(
					    *this,
					    exe_glob);
	  else
	    tcb = new(exe_glob->getSpace()) 
	      ExHbaseAccessInsertTcb(
				     *this,
				     exe_glob);
	}
    }
  else if ((accessType_ == CREATE_) ||
	   (accessType_ == DROP_))
    {
      tcb = new(exe_glob->getSpace()) 
	ExHbaseAccessDDLTcb(
			     *this,
			     exe_glob);
    }
  else if (accessType_ == BULK_LOAD_TASK_)
    {
      tcb = new(exe_glob->getSpace())
        ExHbaseAccessBulkLoadTaskTcb(
                             *this,
                             exe_glob);
    }
  else if ((accessType_ == INIT_MD_) ||
	   (accessType_ == DROP_MD_))
    {
      tcb = new(exe_glob->getSpace()) 
	ExHbaseAccessInitMDTcb(
			     *this,
			     exe_glob);
    }
  else if (accessType_ == GET_TABLES_)
    {
      tcb = new(exe_glob->getSpace()) 
	ExHbaseAccessGetTablesTcb(
			     *this,
			     exe_glob);
    }

  ex_assert(tcb, "Error building ExHbaseAccessTcb.");

  return (tcb);
}

ex_tcb * ExHbaseCoProcAggrTdb::build(ex_globals * glob)
{
  ExExeStmtGlobals * exe_glob = glob->castToExExeStmtGlobals();
  
  ex_assert(exe_glob,"This operator cannot be in DP2");

  ExHbaseCoProcAggrTcb *tcb = NULL;

  tcb = new(exe_glob->getSpace()) 
    ExHbaseCoProcAggrTcb(
			   *this,
			   exe_glob);

  ex_assert(tcb, "Error building ExHbaseAggrTcb.");

  return (tcb);
}

////////////////////////////////////////////////////////////////
// Constructor and initialization.
////////////////////////////////////////////////////////////////

ExHbaseAccessTcb::ExHbaseAccessTcb(
          const ComTdbHbaseAccess &hbaseAccessTdb, 
          ex_globals * glob ) :
  ex_tcb( hbaseAccessTdb, 1, glob)
  , workAtp_(NULL)
  , pool_(NULL)
  , matches_(0)
{
  Space * space = (glob ? glob->getSpace() : 0);
  CollHeap * heap = (glob ? glob->getDefaultHeap() : 0);

  pool_ = new(space) 
        sql_buffer_pool(hbaseAccessTdb.numBuffers_,
        hbaseAccessTdb.bufferSize_,
        space,
        SqlBufferBase::NORMAL_);

  pool_->setStaticMode(TRUE);
        
  // Allocate the queue to communicate with parent
  allocateParentQueues(qparent_);

  if (hbaseAccessTdb.workCriDesc_)
    {
      workAtp_ = allocateAtp(hbaseAccessTdb.workCriDesc_, space);
      if (hbaseAccessTdb.asciiTuppIndex_ > 0)
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.asciiTuppIndex_), 0);
      if (hbaseAccessTdb.convertTuppIndex_ > 0)
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.convertTuppIndex_), 0);
      if (hbaseAccessTdb.updateTuppIndex_ > 0)
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.updateTuppIndex_), 0);
      if (hbaseAccessTdb.mergeInsertTuppIndex_ > 0)
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.mergeInsertTuppIndex_), 0);
      if (hbaseAccessTdb.mergeInsertRowIdTuppIndex_ > 0)
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.mergeInsertRowIdTuppIndex_), 0);
      if (hbaseAccessTdb.rowIdTuppIndex_ > 0)      
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.rowIdTuppIndex_), 0);
      if (hbaseAccessTdb.rowIdAsciiTuppIndex_ > 0)      
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.rowIdAsciiTuppIndex_), 0);
      if (hbaseAccessTdb.keyTuppIndex_ > 0)      
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.keyTuppIndex_), 0);

      if (hbaseAccessTdb.keyColValTuppIndex_ > 0)      
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.keyColValTuppIndex_), 0);

      if (hbaseAccessTdb.hbaseFilterValTuppIndex_ > 0)      
	pool_->get_free_tuple(workAtp_->getTupp(hbaseAccessTdb.hbaseFilterValTuppIndex_), 0);

    }

  keySubsetExeExpr_ = NULL;
  keyMdamExeExpr_ = NULL;
  if (hbaseAccessTdb.keySubsetGen())
    {
      // this constructor will also do the fixups of underlying expressions
      keySubsetExeExpr_ = new(glob->getSpace()) 
	keySingleSubsetEx(*hbaseAccessTdb.keySubsetGen(),
			  1, // version 1
			  pool_,
			  getGlobals(),
			  getExpressionMode(), this);
    }
  else if (hbaseAccessTdb.keyMDAMGen())
    {
      // this constructor will also do the fixups of underlying expressions
      keyMdamExeExpr_ = new(glob->getSpace()) 
	keyMdamEx(*hbaseAccessTdb.keyMDAMGen(),
			  1, // version 1
			  pool_,
			  getGlobals(),
			  this);
    }

  // fixup expressions
  if (convertExpr())
    convertExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (updateExpr())
    updateExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (mergeInsertExpr())
    mergeInsertExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (mergeInsertRowIdExpr())
    mergeInsertRowIdExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (mergeUpdScanExpr())
    mergeUpdScanExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (returnFetchExpr())
    returnFetchExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (returnUpdateExpr())
    returnUpdateExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (returnMergeInsertExpr())
    returnMergeInsertExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (scanExpr())
    scanExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (rowIdExpr())
    rowIdExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (encodedKeyExpr())
    encodedKeyExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (keyColValExpr())
    keyColValExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  if (hbaseFilterValExpr())
    hbaseFilterValExpr()->fixup(0, getExpressionMode(), this,  space, heap, FALSE, glob);
  
  // Register subtasks with the scheduler
  registerSubtasks();
  registerResizeSubtasks();

  int jniDebugPort = 0;
  int jniDebugTimeout = 0;
  ehi_ = ExpHbaseInterface::newInstance(glob->getDefaultHeap(),
					//					(char*)"localhost", 
					(char*)hbaseAccessTdb.server_, 
					(char*)hbaseAccessTdb.port_,
					(char*)hbaseAccessTdb.interface_,
					//                                        (char*)"2181", 
					(char*)hbaseAccessTdb.zkPort_,
                                        jniDebugPort,
                                        jniDebugTimeout);

  asciiRow_ = NULL;
  asciiRowMissingCols_ = NULL;
  latestTimestampForCols_ = NULL;
  convertRow_ = NULL;
  updateRow_ = NULL;
  mergeInsertRow_ = NULL;
  rowIdRow_ = NULL;
  rowwiseRow_ = NULL;
  rowwiseRowLen_ = 0;
  beginRowIdRow_ = NULL;
  endRowIdRow_ = NULL;
  beginKeyRow_ = NULL;
  endKeyRow_ = NULL;
  encodedKeyRow_ = NULL;
  keyColValRow_ = NULL;
  hbaseFilterValRow_ = NULL;

  if (hbaseAccessTdb.asciiRowLen_ > 0)
    {
      asciiRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.asciiRowLen_];
      asciiRowMissingCols_ = 
	new(glob->getDefaultHeap()) 
	char[hbaseAccessTdb.workCriDesc_->getTupleDescriptor(hbaseAccessTdb.asciiTuppIndex_)->numAttrs()];
      latestTimestampForCols_ = new(glob->getDefaultHeap()) 
	long[hbaseAccessTdb.workCriDesc_->getTupleDescriptor(hbaseAccessTdb.asciiTuppIndex_)->numAttrs()] ;
    }

  if (hbaseAccessTdb.convertRowLen_ > 0)
    convertRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.convertRowLen_];

  if (hbaseAccessTdb.updateRowLen_ > 0)
    updateRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.updateRowLen_];

  if (hbaseAccessTdb.mergeInsertRowLen_ > 0)
    mergeInsertRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.mergeInsertRowLen_];

  if (hbaseAccessTdb.rowIdLen_ > 0)
    {
      // +2 for adding a char to exclude the begin/end row, and for null terminator
      rowIdRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.rowIdLen_ + 2];

      beginRowIdRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.rowIdLen_ + 2];

      endRowIdRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.rowIdLen_ + 2];
    }
      
  if (hbaseAccessTdb.convertRowLen_ > 0)
    rowwiseRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.convertRowLen_];
  if (hbaseAccessTdb.rowIdAsciiRowLen_ > 0)
    rowIdAsciiRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.rowIdAsciiRowLen_];

  if (hbaseAccessTdb.keyLen_ > 0)
    {
      beginKeyRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.keyLen_];
      endKeyRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.keyLen_];
      encodedKeyRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.keyLen_];
    }

  if (hbaseAccessTdb.keyColValLen_ > 0)
    {
      keyColValRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.keyColValLen_];
    }

  if (hbaseAccessTdb.hbaseFilterValRowLen_ > 0)
    {
      hbaseFilterValRow_ = new(glob->getDefaultHeap()) char[hbaseAccessTdb.hbaseFilterValRowLen_];
    }

  prevRowIdMaxLen_ = 0;
  prevRowId_.val = NULL;
  prevRowId_.len = 0;
  isEOD_ = FALSE;
}
    
ExHbaseAccessTcb::~ExHbaseAccessTcb()
{
  freeResources();
}

void ExHbaseAccessTcb::freeResources()
{
  if (workAtp_)
  {
    workAtp_->release();
    deallocateAtp(workAtp_, getSpace());
    workAtp_ = NULL;
  }
  if (pool_)
  {
    delete pool_;
    pool_ = NULL;
  }
  if (qparent_.up)
  {
    delete qparent_.up;
    qparent_.up = NULL;
  }
  if (qparent_.down)
  {
    delete qparent_.down;
    qparent_.down = NULL;
  }
  delete ehi_;
}

NABoolean ExHbaseAccessTcb::needStatsEntry()
{
  return TRUE;

  // stats are collected for ALL and OPERATOR options.
  if ((getGlobals()->getStatsArea()->getCollectStatsType() == 
       ComTdb::ALL_STATS) ||
      (getGlobals()->getStatsArea()->getCollectStatsType() == 
      ComTdb::OPERATOR_STATS))
    return TRUE;
  else if ( getGlobals()->getStatsArea()->getCollectStatsType() == ComTdb::PERTABLE_STATS)
    return TRUE;
  else
    return FALSE;
}

ExOperStats * ExHbaseAccessTcb::doAllocateStatsEntry(
                                                        CollHeap *heap,
                                                        ComTdb *tdb)
{
  return new(heap) ExHbaseAccessStats(heap,
				   this,
				   tdb);
}

void ExHbaseAccessTcb::registerSubtasks()
{
  ExScheduler *sched = getGlobals()->getScheduler();

  sched->registerInsertSubtask(sWork,   this, qparent_.down,"PD");
  sched->registerUnblockSubtask(sWork,    this, qparent_.up,  "PU");
  sched->registerCancelSubtask(sWork,     this, qparent_.down,"CN");

}

ex_tcb_private_state *ExHbaseAccessTcb::allocatePstates(
     Lng32 &numElems,      // inout, desired/actual elements
     Lng32 &pstateLength)  // out, length of one element
{
  PstateAllocator<ex_tcb_private_state> pa;
  return pa.allocatePstates(this, numElems, pstateLength);
}

ExWorkProcRetcode ExHbaseAccessTcb::work()
{
  ex_assert(0, "Should not reach ExHbaseAccessTcb::work()");

  return WORK_OK;
}

short ExHbaseAccessTcb::allocateUpEntryTupps
(Lng32 tupp1index, Lng32 tupp1length, Lng32 tupp2index, Lng32 tupp2length,
 NABoolean isVarchar,
 short * rc)
{
  tupp p1;
  tupp p2;
  if (tupp1index > 0)
    {
      if (pool_->get_free_tuple(p1, (Lng32)
				((isVarchar ? SQL_VARCHAR_HDR_SIZE : 0)
				 + tupp1length)))
	{
	  if (rc)
	    *rc = WORK_POOL_BLOCKED;
	  return -1;
	}
    }

  if (tupp2index > 0)
    {
      if (pool_->get_free_tuple(p2, (Lng32)
				((isVarchar ? SQL_VARCHAR_HDR_SIZE : 0)
				 + tupp2length)))
	{
	  if (rc)
	    *rc = WORK_POOL_BLOCKED;
	  return -1;
	}
    }

  ex_queue_entry * pentry_down = qparent_.down->getHeadEntry();
  ex_queue_entry * up_entry = qparent_.up->getTailEntry();

  up_entry->copyAtp(pentry_down);

  if (tupp1index > 0)
    {
      up_entry->getAtp()->getTupp(tupp1index) = p1;
    }

  if (tupp2index > 0)
    {
      up_entry->getAtp()->getTupp(tupp2index) = p2;
    }

  return 0;
}

short ExHbaseAccessTcb::moveRowToUpQueue(short * rc)
{
  if (qparent_.up->isFull())
    {
      if (rc)
	*rc = WORK_OK;
      return -1;
    }

  ex_queue_entry * pentry_down = qparent_.down->getHeadEntry();
  ex_queue_entry * up_entry = qparent_.up->getTailEntry();
  
  up_entry->upState.parentIndex = 
    pentry_down->downState.parentIndex;
  
  up_entry->upState.setMatchNo(++matches_);
  up_entry->upState.status = ex_queue::Q_OK_MMORE;

  // insert into parent
  qparent_.up->insert();

  return 0;
}

short ExHbaseAccessTcb::moveRowToUpQueue(const char * row, Lng32 len, 
					 short * rc, NABoolean isVarchar)
{
  if (qparent_.up->isFull())
    {
      if (rc)
	*rc = WORK_OK;
      return -1;
    }

  Lng32 length;
  if (len <= 0)
    length = strlen(row);
  else
    length = len;

  tupp p;
  if (pool_->get_free_tuple(p, (Lng32)
			    ((isVarchar ? SQL_VARCHAR_HDR_SIZE : 0)
			     + length)))
    {
      if (rc)
	*rc = WORK_POOL_BLOCKED;
      return -1;
    }
  
  char * dp = p.getDataPointer();
  if (isVarchar)
    {
      *(short*)dp = (short)length;
      str_cpy_all(&dp[SQL_VARCHAR_HDR_SIZE], row, length);
    }
  else
    {
      str_cpy_all(dp, row, length);
    }

  ex_queue_entry * pentry_down = qparent_.down->getHeadEntry();
  ex_queue_entry * up_entry = qparent_.up->getTailEntry();
  
  up_entry->copyAtp(pentry_down);
  up_entry->getAtp()->getTupp((Lng32)hbaseAccessTdb().returnedTuppIndex_) = p;

  up_entry->upState.parentIndex = 
    pentry_down->downState.parentIndex;
  
  up_entry->upState.setMatchNo(++matches_);
  up_entry->upState.status = ex_queue::Q_OK_MMORE;

  // insert into parent
  qparent_.up->insert();

  return 0;
}

short ExHbaseAccessTcb::setupError(Lng32 retcode, const char * str, const char * str2)
{
  ContextCli *currContext = GetCliGlobals()->currContext();
  // Make sure retcode is positive.
  if (retcode < 0)
    retcode = -retcode;
    
  if ((ABS(retcode) >= HBASE_MIN_ERROR_NUM) &&
      (ABS(retcode) <= HBASE_MAX_ERROR_NUM))
    {
      ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();
 
      Lng32 cliError = 0;
      
      Lng32 intParam1 = -retcode;
      ComDiagsArea * diagsArea = NULL;
      ExRaiseSqlError(getHeap(), &diagsArea, 
		      (ExeErrorCode)(8448), NULL, &intParam1, 
		      &cliError, NULL, 
		      (str ? (char*)str : (char*)" "),
		      getHbaseErrStr(retcode),
                      (str2 ? (char*)str2 : 
                      (char *)currContext->getJniErrorStr().data())); 
      pentry_down->setDiagsArea(diagsArea);
      return -1;
    }

  return 0;
}

short ExHbaseAccessTcb::handleError(short &rc)
{
  if (qparent_.up->isFull())
    {
      rc = WORK_OK;
      return -1;
    }

  if (qparent_.up->isFull())
    return WORK_OK;
  
  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();
  ex_queue_entry *up_entry = qparent_.up->getTailEntry();
  up_entry->copyAtp(pentry_down);
  up_entry->upState.parentIndex =
    pentry_down->downState.parentIndex;
  up_entry->upState.downIndex = qparent_.down->getHeadIndex();
  up_entry->upState.status = ex_queue::Q_SQLERROR;
  qparent_.up->insert();

  return 0;
}

short ExHbaseAccessTcb::handleDone(ExWorkProcRetcode &rc, Int64 rowsAffected)
{
  if (qparent_.up->isFull())
    {
      rc = WORK_OK;
      return -1;
    }

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();
  ex_queue_entry *up_entry = qparent_.up->getTailEntry();
  up_entry->copyAtp(pentry_down);
  up_entry->upState.parentIndex =
    pentry_down->downState.parentIndex;
  up_entry->upState.downIndex = qparent_.down->getHeadIndex();
  up_entry->upState.status = ex_queue::Q_NO_DATA;
  up_entry->upState.setMatchNo(matches_);
  if (rowsAffected > 0 && hbaseAccessTdb().computeRowsAffected())
    {
      ExMasterStmtGlobals *g = getGlobals()->
        castToExExeStmtGlobals()->castToExMasterStmtGlobals();
      if (g)
        {
          g->setRowsAffected(g->getRowsAffected() + matches_);
        }
      else
        {
          ComDiagsArea * diagsArea = up_entry->getDiagsArea();
          if (!diagsArea)
            {
              diagsArea = 
                ComDiagsArea::allocate(getGlobals()->getDefaultHeap());
              up_entry->setDiagsArea(diagsArea);
            }
          diagsArea->addRowCount(rowsAffected);
        }
    }
  qparent_.up->insert();
  
  qparent_.down->removeHead();

  return 0;
}

// return:
// 0, if all ok.
// -1, if error.
short ExHbaseAccessTcb::createColumnwiseRow()
{
  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ExpTupleDesc * asciiSourceTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().asciiTuppIndex_);
  
  for (Lng32 i = 0; i <  asciiSourceTD->numAttrs(); i++)
    {
      Attributes * attr = asciiSourceTD->getAttr(i);
      if (attr)
	{
	  switch (i)
	    {
	    case HBASE_ROW_ID_INDEX:
	      {
		*(short*)&asciiRow_[attr->getVCLenIndOffset()] = rowId_.len;
		*(Int64*)&asciiRow_[attr->getOffset()] =
		  (Int64)rowId_.val;
	      }
	      break;
	      
	    case HBASE_COL_FAMILY_INDEX:
	      {
		Lng32 i = 0;
		NABoolean done = FALSE;
		while (not done)
		  {
		    if (colName_.val[i] != ':')
		      i++;
		    else
		      done = TRUE;
		  }
		*(short*)&asciiRow_[attr->getVCLenIndOffset()] = i; //colName_.len;
		*(Int64*)&asciiRow_[attr->getOffset()] =
		  (Int64)colName_.val;
	      }
	      break;
	      
	    case HBASE_COL_NAME_INDEX:
	      {
		Lng32 i = 0;
		NABoolean done = FALSE;
		while (not done)
		  {
		    if (colName_.val[i] != ':')
		      i++;
		    else
		      done = TRUE;
		  }

		*(short*)&asciiRow_[attr->getVCLenIndOffset()] = colName_.len - (i+1);
		*(Int64*)&asciiRow_[attr->getOffset()] =
		  (Int64)&colName_.val[i+1];
	      }
	      break;
	      
	    case HBASE_COL_VALUE_INDEX:
	      {
		*(short*)&asciiRow_[attr->getVCLenIndOffset()] = colVal_.len;
		*(Int64*)&asciiRow_[attr->getOffset()] =
		  (Int64)colVal_.val;
	      }
	      break;
	      
	    case HBASE_COL_TS_INDEX:
	      {
		*(Int64*)&asciiRow_[attr->getOffset()] = colTS_;
	      }
	      break;
	      
	    } // switch
	} // if attr
    } // for
  
  workAtp_->getTupp(hbaseAccessTdb().convertTuppIndex_)
    .setDataPointer(convertRow_);
  workAtp_->getTupp(hbaseAccessTdb().asciiTuppIndex_) 
    .setDataPointer(asciiRow_);
  
  if (convertExpr())
    {
      ex_expr::exp_return_type evalRetCode =
	convertExpr()->eval(pentry_down->getAtp(), workAtp_);
      if (evalRetCode == ex_expr::EXPR_ERROR)
	{
	  return -1;
	}
    }

  return 0;
}

// return:
// 0, if all ok.
// -1, if error.
short ExHbaseAccessTcb::createRowwiseRow()
{
  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ExpTupleDesc * asciiSourceTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().asciiTuppIndex_);
  
  for (Lng32 i = 0; i <  asciiSourceTD->numAttrs(); i++)
    {
      Attributes * attr = asciiSourceTD->getAttr(i);
      if (attr)
	{
	  switch (i)
	    {
	    case HBASE_ROW_ROWID_INDEX:
	      {
		*(short*)&asciiRow_[attr->getVCLenIndOffset()] = prevRowId_.len;
		*(Int64*)&asciiRow_[attr->getOffset()] =
		  (Int64)prevRowId_.val;
	      }
	      break;
	      
	    case HBASE_COL_DETAILS_INDEX:
	      {
		//		*(short*)&asciiRow_[attr->getVCLenIndOffset()] = strlen(rowwiseRow_);
		*(short*)&asciiRow_[attr->getVCLenIndOffset()] = rowwiseRowLen_;
		*(Int64*)&asciiRow_[attr->getOffset()] =
		  (Int64)rowwiseRow_;
	      }
	      break;
	      
	    } // switch
	} // if
    } // for
  
  workAtp_->getTupp(hbaseAccessTdb().convertTuppIndex_)
    .setDataPointer(convertRow_);
  workAtp_->getTupp(hbaseAccessTdb().asciiTuppIndex_) 
    .setDataPointer(asciiRow_);
  
  if (convertExpr())
    {
      ex_expr::exp_return_type evalRetCode =
	convertExpr()->eval(pentry_down->getAtp(), workAtp_);
      if (evalRetCode == ex_expr::EXPR_ERROR)
	{
	  return -1;
	}
    }

  return 0;
}

// return 1, if found. 0, if not found. -1, if error.
// Columns retrieved from hbase are in sorted order.
// Columns in listOfColNames may or may not be in sorted order.
// This method advances LOCN until a match is found or a full
// loop of LOCN is done.
// If cols in LOCN are in sorted order, then the search will be faster.
// 
short ExHbaseAccessTcb::getColPos(char * colName, Lng32 colNameLen, Lng32 &idx)
{
  NABoolean found = FALSE;

  Lng32 numCompares = hbaseAccessTdb().listOfFetchedColNames()->numEntries();
  while ((numCompares > 0) &&
            (NOT found))
    {
      if (hbaseAccessTdb().listOfFetchedColNames()->atEnd())
      {
	hbaseAccessTdb().listOfFetchedColNames()->position();
        idx = -1;
      }
      short len = *(short*)hbaseAccessTdb().listOfFetchedColNames()->getCurr();
      char * currName = 
	&((char*)hbaseAccessTdb().listOfFetchedColNames()->getCurr())[sizeof(short)];

      if ((colNameLen == len) &&
	  (memcmp(colName, currName, len) == 0))
	{
	  found = TRUE;
	}
      else
	numCompares--;

      hbaseAccessTdb().listOfFetchedColNames()->advance();
      idx++;
    }
  
  if (found)
    return 1;
  else
    return 0;
}

// return:
// 0, if all ok.
// -1, if error.
short ExHbaseAccessTcb::createSQRow(TRowResult &rowResult)
{
  // no columns are being fetched from hbase, do not create a row.
  if (hbaseAccessTdb().listOfFetchedColNames()->numEntries() == 0)
    return 0;

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ExpTupleDesc * asciiSourceTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().asciiTuppIndex_);

  ExpTupleDesc * convertTuppTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().convertTuppIndex_);
  
  Attributes * attr = NULL;

  // initialize as missing cols.
  // TBD: can optimize to skip this step if there are no nullable and no added cols
  memset(asciiRowMissingCols_, 1, asciiSourceTD->numAttrs());
  
  hbaseAccessTdb().listOfFetchedColNames()->position();
  Lng32 idx = -1;

  CellMap::const_iterator colIter = rowResult.columns.begin();

  while (colIter != rowResult.columns.end())
    {
      char * colName = (char*)colIter->first.data();
      Lng32 colNameLen = colIter->first.length();

      char * colVal = (char*)colIter->second.value.data();
      Lng32 colValLen = colIter->second.value.length();

      if (! getColPos(colName, colNameLen, idx)) // not found
	{
	  // error
	  return -HBASE_CREATE_ROW_ERROR;
	}

      // not missing any more
      asciiRowMissingCols_[idx] = 0;

      Attributes * attr = asciiSourceTD->getAttr(idx);
      if (! attr)
	{
	  // error
	  return -HBASE_CREATE_ROW_ERROR;
	}

      if (attr->getNullFlag())
	{
	  if (*colVal)
	    *(short*)&asciiRow_[attr->getNullIndOffset()] = -1;
	  else
	    *(short*)&asciiRow_[attr->getNullIndOffset()] = 0;

	  colValLen = colValLen - sizeof(char);
	  colVal += sizeof(char);
	}

      if (attr->getVCIndicatorLength() > 0)
	{
	  if (attr->getVCIndicatorLength() == sizeof(short))
	    *(short*)&asciiRow_[attr->getVCLenIndOffset()] = colValLen; 
	  else
	    *(Lng32*)&asciiRow_[attr->getVCLenIndOffset()] = colValLen; 
	  *(Int64*)&asciiRow_[attr->getOffset()] = (Int64)colVal;
	}
      else
	{
	  char * srcPtr = &asciiRow_[attr->getOffset()];
	  Lng32 copyLen = MINOF(attr->getLength(), colValLen);
	  str_cpy_all(srcPtr, colVal, copyLen);
	}

      colIter++;
    }

  // fill in null or default values for missing cols.
  for (idx = 0; idx < asciiSourceTD->numAttrs(); idx++)
    {
      if (asciiRowMissingCols_[idx] == 1) // missing
	{
	  attr = asciiSourceTD->getAttr(idx);
	  if (! attr)
	    {
	      // error
	      return -HBASE_CREATE_ROW_ERROR;
	    }
	  
	  char * defVal = attr->getDefaultValue();
	  char * defValPtr = defVal;
	  short nullVal = 0;
	  if (attr->getNullFlag())
	    {
	      nullVal = *(short*)defVal;
	      *(short*)&asciiRow_[attr->getNullIndOffset()] = nullVal;
	      
	      defValPtr += 2;
	    }
	  
	  if (! nullVal)
	    {
	      if (attr->getVCIndicatorLength() > 0)
		{
		  Lng32 vcLen = *(short*)defValPtr;
		  if (attr->getVCIndicatorLength() == sizeof(short))
		    *(short*)&asciiRow_[attr->getVCLenIndOffset()] = vcLen; 
		  else
		    *(Lng32*)&asciiRow_[attr->getVCLenIndOffset()] = vcLen;
		  
		  defValPtr += attr->getVCIndicatorLength();
		  
		  *(Int64*)&asciiRow_[attr->getOffset()] = (Int64)defValPtr;
		}
	      else
		{
		  char * srcPtr = &asciiRow_[attr->getOffset()];
		  Lng32 copyLen = attr->getLength();
		  str_cpy_all(srcPtr, defValPtr, copyLen);
		}
	    } // not nullVal
	} // missing col
    }

  workAtp_->getTupp(hbaseAccessTdb().convertTuppIndex_)
    .setDataPointer(convertRow_);
  workAtp_->getTupp(hbaseAccessTdb().asciiTuppIndex_) 
    .setDataPointer(asciiRow_);
  
  if (convertExpr())
    {
      ex_expr::exp_return_type evalRetCode =
	convertExpr()->eval(pentry_down->getAtp(), workAtp_);
      if (evalRetCode == ex_expr::EXPR_ERROR)
	{
	  return -1;
	}
    }

  return 0;
}
#define INLINE_COLNAME_LEN 256
short ExHbaseAccessTcb::createSQRow(jbyte *rowResult)
{
  // no columns are being fetched from hbase, do not create a row.
  if (hbaseAccessTdb().listOfFetchedColNames()->numEntries() == 0)
    return 0;

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ExpTupleDesc * asciiSourceTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().asciiTuppIndex_);

  ExpTupleDesc * convertTuppTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().convertTuppIndex_);
  
  Attributes * attr = NULL;

  // initialize as missing cols.
  // TBD: can optimize to skip this step if there are no nullable and no added cols
  memset(asciiRowMissingCols_, 1, asciiSourceTD->numAttrs());
    // initialize latest timestamp to 0 for every column
  memset(latestTimestampForCols_, 0, (asciiSourceTD->numAttrs()*sizeof(long)));
  
  hbaseAccessTdb().listOfFetchedColNames()->position();
  Lng32 idx = -1;

  Int32 kvLength, valueLength, valueOffset, qualLength, qualOffset;
  Int32 familyLength, familyOffset;
  long timestamp;

  char *kvBuf = (char *) rowResult;

  Int32 numCols = *(Int32 *)kvBuf;
  kvBuf += sizeof(numCols);
  if (numCols == 0)
    return 0;

  Int32 rowIDLen = *(Int32 *)kvBuf;
  kvBuf += sizeof(rowIDLen);
  kvBuf += rowIDLen;

  Int32 *temp;
  char *value;
  char *buffer;
  char *colName;
  char *family;
  char inlineColName[INLINE_COLNAME_LEN+1];
  char *fullColName;
  char *colVal; 
  Lng32 colValLen;
  Lng32  colNameLen;

  for (int i= 0; i< numCols; i++)
  {
     temp = (Int32 *)kvBuf;
     kvLength = *temp++;
     valueLength = *temp++;
     valueOffset = *temp++;
     qualLength = *temp++;
     qualOffset = *temp++;
     familyLength = *temp++;
     familyOffset = *temp++;
     timestamp = *(long *)temp;
     temp += 2;
     buffer = (char *)temp;
     value = buffer + valueOffset; 

     colName = (char*)buffer + qualOffset;
     family = (char *)buffer + familyOffset;
     colNameLen = familyLength + qualLength + 1; // 1 for ':'

     if (colNameLen > INLINE_COLNAME_LEN)
        fullColName = new (getHeap()) char[colNameLen + 1];
     else
        fullColName = inlineColName;
     strncpy(fullColName, family, familyLength);
     fullColName[familyLength] = '\0';
     strcat(fullColName, ":");
     strncat(fullColName, colName, qualLength); 
     fullColName[colNameLen] = '\0';
    
     colName = fullColName;
      
     colVal = (char*)value;
     colValLen = valueLength;

      if (! getColPos(colName, colNameLen, idx)) // not found
	{
          if (colNameLen > INLINE_COLNAME_LEN)
             NADELETEBASIC(fullColName, getHeap());
	  // error
	  return -HBASE_CREATE_ROW_ERROR;
	}

      // not missing any more
      asciiRowMissingCols_[idx] = 0;
      if (timestamp > latestTimestampForCols_[idx])
        latestTimestampForCols_[idx] = timestamp;

      Attributes * attr = asciiSourceTD->getAttr(idx);
      if (! attr)
	{
          if (colNameLen > INLINE_COLNAME_LEN)
              NADELETEBASIC(fullColName, getHeap());
	  // error
	  return -HBASE_CREATE_ROW_ERROR;
	}

      // copy to asciiRow only if this is the latest version seen so far
      // for this column. On 6/10/2014 we get two versions for a newly
      // updated column that has not been committed yet.
      if (timestamp == latestTimestampForCols_[idx]) 
        {
          if (attr->getNullFlag())
            {
              if (*colVal)
                *(short*)&asciiRow_[attr->getNullIndOffset()] = -1;
              else
                *(short*)&asciiRow_[attr->getNullIndOffset()] = 0;

              colValLen = colValLen - sizeof(char);
              colVal += sizeof(char);
            }

          if (attr->getVCIndicatorLength() > 0)
            {
              if (attr->getVCIndicatorLength() == sizeof(short))
                *(short*)&asciiRow_[attr->getVCLenIndOffset()] = colValLen; 
              else
                *(Lng32*)&asciiRow_[attr->getVCLenIndOffset()] = colValLen; 
              *(Int64*)&asciiRow_[attr->getOffset()] = (Int64)colVal;
            }
          else
            {
              char * srcPtr = &asciiRow_[attr->getOffset()];
              Lng32 copyLen = MINOF(attr->getLength(), colValLen);
              str_cpy_all(srcPtr, colVal, copyLen);
            }
        }
    if (colNameLen > INLINE_COLNAME_LEN)
       NADELETEBASIC(fullColName, getHeap());
    kvBuf = (char *)temp + kvLength;
  }

  // fill in null or default values for missing cols.
  for (idx = 0; idx < asciiSourceTD->numAttrs(); idx++)
    {
      if (asciiRowMissingCols_[idx] == 1) // missing
	{
	  attr = asciiSourceTD->getAttr(idx);
	  if (! attr)
	    {
	      // error
	      return -HBASE_CREATE_ROW_ERROR;
	    }
	  
	  char * defVal = attr->getDefaultValue();
	  char * defValPtr = defVal;
	  short nullVal = 0;
	  if (attr->getNullFlag())
	    {
	      nullVal = *(short*)defVal;
	      *(short*)&asciiRow_[attr->getNullIndOffset()] = nullVal;
	      
	      defValPtr += 2;
	    }
	  
	  if (! nullVal)
	    {
	      if (attr->getVCIndicatorLength() > 0)
		{
		  Lng32 vcLen = *(short*)defValPtr;
		  if (attr->getVCIndicatorLength() == sizeof(short))
		    *(short*)&asciiRow_[attr->getVCLenIndOffset()] = vcLen; 
		  else
		    *(Lng32*)&asciiRow_[attr->getVCLenIndOffset()] = vcLen;
		  
		  defValPtr += attr->getVCIndicatorLength();
		  
		  *(Int64*)&asciiRow_[attr->getOffset()] = (Int64)defValPtr;
		}
	      else
		{
		  char * srcPtr = &asciiRow_[attr->getOffset()];
		  Lng32 copyLen = attr->getLength();
		  str_cpy_all(srcPtr, defValPtr, copyLen);
		}
	    } // not nullVal
	} // missing col
    }

  workAtp_->getTupp(hbaseAccessTdb().convertTuppIndex_)
    .setDataPointer(convertRow_);
  workAtp_->getTupp(hbaseAccessTdb().asciiTuppIndex_) 
    .setDataPointer(asciiRow_);
  
  if (convertExpr())
    {
      ex_expr::exp_return_type evalRetCode =
	convertExpr()->eval(pentry_down->getAtp(), workAtp_);
      if (evalRetCode == ex_expr::EXPR_ERROR)
	{
	  return -1;
	}
    }

  return 0;
}

// returns:

// returns:
// 0, if expr is false
// 1, if no pred or expr is true
// -1, if expr error
short ExHbaseAccessTcb::applyPred(ex_expr * expr, UInt16 tuppIndex,
				  char * tuppRow)
{
  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  if (! expr)
    {
      return 1;
    }
  
  ex_expr::exp_return_type exprRetCode = ex_expr::EXPR_OK;
  
  if ((tuppRow) && (tuppIndex > 0))
    workAtp_->getTupp(tuppIndex)
      .setDataPointer(tuppRow);
  else
    workAtp_->getTupp(hbaseAccessTdb().convertTuppIndex_)
      .setDataPointer(convertRow_);
  
  exprRetCode =
    expr->eval(pentry_down->getAtp(), workAtp_);
  
  if (exprRetCode == ex_expr::EXPR_ERROR)
    {
      return -1;
    }
  
  if (exprRetCode == ex_expr::EXPR_TRUE)
    {
      return 1;
    }
  
  return 0;
}

short ExHbaseAccessTcb::extractColFamilyAndName(char * input, 
						Text &colFam, Text &colName)
{
  return ExFunctionHbaseColumnLookup::extractColFamilyAndName(input, 
							      TRUE,
							      colFam, colName);
}

short ExHbaseAccessTcb::evalKeyColValExpr(Text &columnToCheck, Text &colValToCheck)
{
  if (! keyColValExpr())
    return -1;

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ex_expr::exp_return_type exprRetCode = ex_expr::EXPR_OK;
  
  workAtp_->getTupp(hbaseAccessTdb().keyColValTuppIndex_)
    .setDataPointer(keyColValRow_);
  
  exprRetCode =
    keyColValExpr()->eval(pentry_down->getAtp(), workAtp_);
  
  if (exprRetCode == ex_expr::EXPR_ERROR)
    {
      return -1;
    }
  
  colValToCheck.assign(keyColValRow_, hbaseAccessTdb().keyColValLen_);

  char * keyColNamePtr = hbaseAccessTdb().keyColName();
  short nameLen = *(short*)keyColNamePtr;
  char * keyColName = &keyColNamePtr[sizeof(short)];
  columnToCheck.assign(keyColName, nameLen);

  return 0;
}

short ExHbaseAccessTcb::evalEncodedKeyExpr()
{
  if (! encodedKeyExpr())
    return 0;

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ex_expr::exp_return_type exprRetCode = ex_expr::EXPR_OK;
  
  workAtp_->getTupp(hbaseAccessTdb().keyTuppIndex_)
    .setDataPointer(encodedKeyRow_);
  
  exprRetCode =
    encodedKeyExpr()->eval(pentry_down->getAtp(), workAtp_);
  
  if (exprRetCode == ex_expr::EXPR_ERROR)
    {
      return -1;
    }
  
  return 0;
}

short ExHbaseAccessTcb::evalRowIdExpr(NABoolean noVarchar)
{
  if (! rowIdExpr())
    return 0;

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ex_expr::exp_return_type exprRetCode = ex_expr::EXPR_OK;
  
  workAtp_->getTupp(hbaseAccessTdb().rowIdTuppIndex_)
    .setDataPointer(rowIdRow_);
  
  exprRetCode =
    rowIdExpr()->eval(pentry_down->getAtp(), workAtp_);
  
  if (exprRetCode == ex_expr::EXPR_ERROR)
    {
      return -1;
    }
  
  if (NOT noVarchar)
    {
      rowId_.val = &rowIdRow_[SQL_VARCHAR_HDR_SIZE];
      rowId_.len = *(short*)rowIdRow_;
    }
  else
   {
     rowId_.val = rowIdRow_;
     rowId_.len = hbaseAccessTdb().rowIdLen_;
    }
    
  return 0;
}

short ExHbaseAccessTcb::evalRowIdAsciiExpr(const char * inputRowIdVals,
					   char * rowIdBuf, // input: buffer where rowid is created
					   char* &outputRowIdPtr,  // output: ptr to rowid.
					   Lng32 excludeKey,
					   Lng32 &outputRowIdLen)
{
  if (! rowIdExpr())
    return -1;

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ex_expr::exp_return_type exprRetCode = ex_expr::EXPR_OK;
  
  ExpTupleDesc * asciiSourceTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().rowIdAsciiTuppIndex_);
  
  Lng32 currPos = 0;
  for (CollIndex i = 0; i < asciiSourceTD->numAttrs(); i++)
    {
      Attributes * attr = asciiSourceTD->getAttr(i);
      if (! attr)
	{
	  // error
	  return -1;
	}
  
      short inputRowIdValLen = *(short*)&inputRowIdVals[currPos];
      currPos += sizeof(short);

      short nullVal = 0;
      if (attr->getNullFlag())
	{
	  inputRowIdValLen -= sizeof(short);
	  
	  nullVal = *(short*)&inputRowIdVals[currPos];
	  
	  if (nullVal)
	    {
	      *(short*)&rowIdAsciiRow_[attr->getNullIndOffset()] = -1;
	    }
	  else
	    {
	      *(short*)&rowIdAsciiRow_[attr->getNullIndOffset()] = 0;
	    }
	  
	  currPos += sizeof(short);
	}
      
      const char * inputRowIdVal = &inputRowIdVals[currPos];
      
      if (attr->getVCIndicatorLength() > 0)
	{
	  *(short*)&rowIdAsciiRow_[attr->getVCLenIndOffset()] = inputRowIdValLen;
	  *(Int64*)&rowIdAsciiRow_[attr->getOffset()] = (Int64)inputRowIdVal;
	}
      else
	{
	  char * srcPtr = &rowIdAsciiRow_[attr->getOffset()];
	  Lng32 copyLen = MINOF(attr->getLength(), inputRowIdValLen);
	  str_cpy_all(srcPtr, (char*)inputRowIdVal, copyLen);
	}
      
      currPos += inputRowIdValLen;
    }
  
  workAtp_->getTupp(hbaseAccessTdb().rowIdTuppIndex_)
    .setDataPointer(rowIdBuf);
  workAtp_->getTupp(hbaseAccessTdb().rowIdAsciiTuppIndex_)
    .setDataPointer(rowIdAsciiRow_);
  
  exprRetCode =
    rowIdExpr()->eval(pentry_down->getAtp(), workAtp_);
  
  if (exprRetCode == ex_expr::EXPR_ERROR)
    {
      return -1;
    }

  // For non-SQ tables,
  // rowid is created as a null terminated string in rowIdBuf. (ANSI char type)
  // At this point, there are 2 extra bytes allocated in the beginning. Need to
  // figure out why that is and if it is needed.
  // Skip the first 2 bytes and point to the actual row.
  // If exclude flag is set, then add a null byte and increment the length by 1.
  if (hbaseAccessTdb().sqHbaseTable())
    {
      outputRowIdPtr = rowIdBuf; 
      outputRowIdLen = hbaseAccessTdb().rowIdLen();
    }
  else
    {
      outputRowIdPtr = &rowIdBuf[2]; 
      outputRowIdLen = strlen(outputRowIdPtr);
    }

  if (excludeKey)
    {
      outputRowIdPtr[outputRowIdLen] = '\0';
      outputRowIdLen++;
    }

  return 0;
}

void ExHbaseAccessTcb::setupPrevRowId()
{
  if (prevRowIdMaxLen_ < rowId_.len)
    {
      if (prevRowId_.val)
	NADELETEBASIC(prevRowId_.val, getHeap());
      prevRowIdMaxLen_ = rowId_.len;
      prevRowId_.val = new(getHeap())  char[prevRowIdMaxLen_];
    }
  
  str_cpy_all(prevRowId_.val, rowId_.val, rowId_.len);
  prevRowId_.len = rowId_.len;
  if (rowwiseRow_)
    {
      //      rowwiseRow_[0] = 0;
      rowwiseRowLen_ = 0;
    }
}  

Lng32 ExHbaseAccessTcb::setupListOfColNames(Queue * listOfColNames,
						   TextVec &columns)
{
  if (listOfColNames)
    {
      listOfColNames->position();
      columns.clear();
      for (Lng32 i = 0; i < listOfColNames->numEntries(); i++)
	{
	  short colNameLen = *(short*)listOfColNames->getCurr();
	  const char * colName = &((char*)listOfColNames->getCurr())[sizeof(short)];
	  
	  listOfColNames->advance();
	  
	  Text colNameText(colName, colNameLen);
	  columns.push_back(colNameText);
	}
    }
  
  return 0;
}

Lng32 ExHbaseAccessTcb::setupUniqueRowIdsAndCols
(ComTdbHbaseAccess::HbaseGetRows*hgr)
{
  if ((! hgr->rowIds()) ||
      (hgr->rowIds()->numEntries() == 0))
    {
      setupError(-HBASE_OPEN_ERROR, "", "RowId list is empty");
      return -1;
    }
  
  hgr->rowIds()->position();
  rowIds_.clear();
  for (Lng32 i = 0; i < hgr->rowIds()->numEntries(); i++)
    {
      if (! rowIdExpr())
	{
	  setupError(-HBASE_OPEN_ERROR, "", "RowId Expr is empty");
	  return -1;
	}
      
      const char * inputRowIdVals = (char*)hgr->rowIds()->getNext();
      char * rowIdRow = NULL;
      if (hgr->rowIds()->numEntries() == 1)
	{
	  rowIdRow = rowIdRow_;
	}
      else
	{
	  // need to be deleted. TBD.
	  rowIdRow = new(getGlobals()->getDefaultHeap()) char[hbaseAccessTdb().rowIdLen_ + 2];
	}
      
      Lng32 rowIdLen = 0;
      char * rowIdPtr = NULL;
      if (evalRowIdAsciiExpr(inputRowIdVals, 
			     rowIdRow, 
			     rowIdPtr,
			     0, 
			     rowIdLen) == -1)
	{
	  return -1;
	}
      
      Text rowIdRowText;
      rowIdRowText.assign(rowIdPtr, rowIdLen);
      rowIds_.push_back(rowIdRowText);
    }

  setupListOfColNames(hbaseAccessTdb().listOfFetchedColNames(), columns_);

  return 0;
}

// sets up beginRowId_, endRowId_ and columns_ fields.
Lng32 ExHbaseAccessTcb::setupSubsetRowIdsAndCols
(ComTdbHbaseAccess::HbaseScanRows* hsr)
{
  Lng32 rowIdLen;
  char * rowIdPtr = NULL;

  const char * inputBeginRowIdVals = (char*)hsr->beginRowId_;
  rowIdLen = *(short*)inputBeginRowIdVals;
  if (rowIdLen > 0)
    {
      if (evalRowIdAsciiExpr(inputBeginRowIdVals, 
			     beginRowIdRow_,
			     rowIdPtr,
			     hsr->beginKeyExclusive_, 
			     rowIdLen) == -1)
	{
	  return -1;
	}

      beginRowId_.assign(rowIdPtr, rowIdLen);
    }
  else
    {
      beginRowId_.assign(hsr->beginRowId_, rowIdLen);
    }
  
  const char * inputEndRowIdVals = (char*)hsr->endRowId_;
  rowIdLen = *(short*)inputEndRowIdVals;
  if (rowIdLen > 0)
    {
      if (evalRowIdAsciiExpr(inputEndRowIdVals, 
			     endRowIdRow_,
			     rowIdPtr,
			     (!hsr->endKeyExclusive_), 
			     rowIdLen) == -1)
	{
	  return -1;
	}
      
      endRowId_.assign(rowIdPtr, rowIdLen);
    }
  else
    {
      endRowId_.assign(hsr->endRowId_, rowIdLen);
    }

  setupListOfColNames(hbaseAccessTdb().listOfFetchedColNames(), columns_);

  return 0;
}

Lng32 ExHbaseAccessTcb::initNextKeyRange(sql_buffer_pool *pool,
			        	 atp_struct * atp)
{
  if (keyExeExpr())
    keyExeExpr()->initNextKeyRange(pool, atp);
  else
    return -1;

  return 0;
}

Lng32 ExHbaseAccessTcb::setupUniqueKeyAndCols(NABoolean doInit)
{
  if (doInit)
    rowIds_.clear();

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  ex_expr::exp_return_type exprRetCode = ex_expr::EXPR_OK;
  keyRangeEx::getNextKeyRangeReturnType keyRangeStatus;

  initNextKeyRange();

  keyRangeStatus = 
    keySubsetExeExpr_->getNextKeyRange(pentry_down->getAtp(), FALSE, TRUE);

  if (keyRangeStatus == keyRangeEx::EXPRESSION_ERROR)
    {
      return -1;
    }

  tupp &keyData = keySubsetExeExpr_->getBkData();
  char * beginKeyRow = keyData.getDataPointer();

  Text rowIdRowText;
  if (hbaseAccessTdb().sqHbaseTable())
    rowIdRowText.assign(beginKeyRow, hbaseAccessTdb().keyLen_);
  else
    {
      // hbase table. Key is in varchar format.
      short keyLen = *(short*)beginKeyRow;
      rowIdRowText.assign(beginKeyRow + sizeof(short), keyLen);
    }

  if (keyRangeStatus == keyRangeEx::NO_MORE_RANGES)
    rowIdRowText.append(1, '\0');

  rowIds_.push_back(rowIdRowText);

  if (doInit)
    setupListOfColNames(hbaseAccessTdb().listOfFetchedColNames(), columns_);

  return 0;
}

keyRangeEx::getNextKeyRangeReturnType ExHbaseAccessTcb::setupSubsetKeys
(NABoolean fetchRangeHadRows)
{
  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();
  ex_expr::exp_return_type exprRetCode = ex_expr::EXPR_OK;
  keyRangeEx::getNextKeyRangeReturnType keyRangeStatus;
  
  keyRangeStatus = 
    keyExeExpr()->getNextKeyRange(pentry_down->getAtp(), fetchRangeHadRows, 
				  (hbaseAccessTdb().sqHbaseTable() ? TRUE : FALSE));
  
  if (keyRangeStatus == keyRangeEx::EXPRESSION_ERROR)
    {
      return keyRangeEx::EXPRESSION_ERROR;
    }

  tupp &beginKeyData = keyExeExpr()->getBkData();
  char * beginKeyRow = beginKeyData.getDataPointer();

  tupp &endKeyData = keyExeExpr()->getEkData();
  char * endKeyRow = endKeyData.getDataPointer();

  short beginKeyLen = hbaseAccessTdb().keyLen_;
  short endKeyLen = hbaseAccessTdb().keyLen_;

  if (NOT hbaseAccessTdb().sqHbaseTable())
    {
      // key in varchar formar
      beginKeyLen = *(short*)beginKeyRow;
      beginKeyRow += sizeof(short);
      if (beginKeyRow[0] == '\0')
	beginKeyLen = 1;
      
      endKeyLen = *(short*)endKeyRow;
      endKeyRow += sizeof(short);
      if (endKeyRow[0] == '\0')
	endKeyLen = 1;
    }

  if (keyRangeStatus == keyRangeEx::NO_MORE_RANGES)
    {
      memset(beginKeyRow, '\377',  beginKeyLen);
      beginRowId_.assign(beginKeyRow, beginKeyLen);
      beginRowId_.append(1, '\0');

      memset(endKeyRow, '\0',  endKeyLen);
      endRowId_.assign(endKeyRow, endKeyLen);

      return keyRangeEx::NO_MORE_RANGES;
    }
  else
    {
      beginRowId_.assign(beginKeyRow, beginKeyLen);
      endRowId_.assign(endKeyRow, endKeyLen);
      
      if (keyExeExpr()->getBkExcludeFlag())
	beginRowId_.append(1, '\0');
      
      if (! keyExeExpr()->getEkExcludeFlag())
	endRowId_.append(1, '\0');
    }

  return keyRangeStatus;
}

Lng32 ExHbaseAccessTcb::setupSubsetKeysAndCols()
{
  initNextKeyRange();
  
  if (setupSubsetKeys() == keyRangeEx::EXPRESSION_ERROR)
    return -1;

  setupListOfColNames(hbaseAccessTdb().listOfFetchedColNames(), columns_);

  return 0;
}

short ExHbaseAccessTcb::createMutations(MutationVec &mutations,
					UInt16 tuppIndex, char * tuppRow,
					Queue * listOfColNames, NABoolean isUpdate,
					std::vector<UInt32> * posVec )
{
  mutations.clear();
  
  ExpTupleDesc * rowTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (tuppIndex);
  
  listOfColNames->position();
  for (Lng32 i = 0; i <  rowTD->numAttrs(); i++)
    {
    Attributes * attr;
      if (!posVec)
        attr = rowTD->getAttr(i);
      else
      {
        attr = rowTD->getAttr((*posVec)[i] -1);
      }

      if (attr)
	{
        short colNameLen;
        char * colName;
         if (!posVec)
         {
	   colNameLen = *(short*)listOfColNames->getCurr();
	   colName = &((char*)listOfColNames->getCurr())[sizeof(short)];
         }
         else
         {
           char * str = (char*)listOfColNames->getCurr();
           colNameLen = *(short*)(str + sizeof(UInt32));
           colName = str + sizeof(short) + sizeof(UInt32);
         }

	  char * colVal = &tuppRow[attr->getOffset()];

	  NABoolean prependNullVal = FALSE;
	  short nullVal = 0;
	  if (attr->getNullFlag())
	    {
	      nullVal = *(short*)&tuppRow[attr->getNullIndOffset()];

	      if ((attr->getDefaultClass() != Attributes::DEFAULT_NULL) &&
		  (nullVal))
		prependNullVal = TRUE;
	      else if (isUpdate && nullVal)
		prependNullVal = TRUE;
	      else if (! nullVal)
		prependNullVal = TRUE;

	      if ((NOT prependNullVal) && (nullVal))
		goto label_end;
	    }
	  
	  mutations.push_back(Mutation());

	  mutations.back().column.assign(colName, colNameLen); 

	  if (prependNullVal)
	    {
	      char nullValChar = 0;
	      if (nullVal)
		nullValChar = -1;
	      mutations.back().value.assign((char*)&nullValChar, sizeof(char));
	      mutations.back().value.append
		(colVal, 
		 attr->getLength(&tuppRow[attr->getVCLenIndOffset()]));
	    }
	  else
	    {
	      mutations.back().value.assign
		(colVal, 
		 attr->getLength(&tuppRow[attr->getVCLenIndOffset()]));
	    }
	} // if attr
      else
	{
	  return -1;
	}
    label_end:		  
      listOfColNames->advance();
    }	// for
  
  return 0;
}

short ExHbaseAccessTcb::createRowwiseMutations(MutationVec &mutations,
					       char * inputRow)
{
  mutations.clear();

  short numEntries;
  str_cpy_all((char*)&numEntries, inputRow, sizeof(short));
  inputRow += sizeof(short);
  
  Text insColNam;
  Text insColVal;
  for (Lng32 ij = 0; ij < numEntries; ij++)
    {
      short colNameLen = *(short*)inputRow;
      inputRow += sizeof(short);
      insColNam.assign(inputRow, colNameLen);
      inputRow += ROUND2(colNameLen);
      
      short nullable = *(short*)inputRow;
      inputRow += sizeof(short);
      
      short colValLen = *(short*)inputRow;
      inputRow += sizeof(short);
      insColVal.assign(inputRow, colValLen);
      inputRow += ROUND2(colValLen);

      if (! nullable)
	{
	  mutations.push_back(Mutation());			      
	  mutations.back().column.append(insColNam);	    
	  mutations.back().value = insColVal;
	}
    }
  
  return 0;
}

short ExHbaseAccessTcb::setupHbaseFilterPreds()
{
  if ((!hbaseAccessTdb().listOfHbaseFilterColNames()) ||
      (hbaseAccessTdb().listOfHbaseFilterColNames()->numEntries() == 0))
    return 0;

  if (! hbaseFilterValExpr())
    return 0;

  ex_queue_entry *pentry_down = qparent_.down->getHeadEntry();

  workAtp_->getTupp(hbaseAccessTdb().hbaseFilterValTuppIndex_)
    .setDataPointer(hbaseFilterValRow_);
  
  ex_expr::exp_return_type evalRetCode =
    hbaseFilterValExpr()->eval(pentry_down->getAtp(), workAtp_);
  if (evalRetCode == ex_expr::EXPR_ERROR)
    {
      return -1;
    }

  ExpTupleDesc * hfrTD =
    hbaseAccessTdb().workCriDesc_->getTupleDescriptor
    (hbaseAccessTdb().hbaseFilterValTuppIndex_);
  
  hbaseFilterValues_.clear();
  for (Lng32 i = 0; i <  hfrTD->numAttrs(); i++)
    {
      Attributes * attr = hfrTD->getAttr(i);
  
      if (attr)
	{
	  Text value;
	  if (attr->getNullFlag())
	    {
	      char nullValChar = 0;

	      short nullVal = *(short*)&hbaseFilterValRow_[attr->getNullIndOffset()];

	      if (nullVal)
		nullValChar = -1;
	      value.assign((char*)&nullValChar, sizeof(char));
	    }	  

	  char * colVal = &hbaseFilterValRow_[attr->getOffset()];

	  value.append(colVal,
		       attr->getLength(&hbaseFilterValRow_[attr->getVCLenIndOffset()]));

	  hbaseFilterValues_.push_back(value);
	}
    }

  setupListOfColNames(hbaseAccessTdb().listOfHbaseFilterColNames(),
		      hbaseFilterColumns_);

  hbaseFilterOps_.clear();
  hbaseAccessTdb().listOfHbaseCompareOps()->position();
  while (NOT hbaseAccessTdb().listOfHbaseCompareOps()->atEnd())
    {
      char * op = (char*)hbaseAccessTdb().listOfHbaseCompareOps()->getCurr();

      hbaseFilterOps_.push_back(op);
      hbaseAccessTdb().listOfHbaseCompareOps()->advance();
    }

  return 0;
}

ExHbaseTaskTcb::ExHbaseTaskTcb(ExHbaseAccessTcb * tcb)
  : tcb_(tcb)
{}

ExWorkProcRetcode ExHbaseTaskTcb::work(short &rc)
{
  ex_assert(0, "Should not reach ExHbaseTaskTcb::work()");

  return WORK_OK;
}

