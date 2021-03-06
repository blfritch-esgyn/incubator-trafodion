# @@@ START COPYRIGHT @@@
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
# @@@ END COPYRIGHT @@@

include ./macros.gmk


# set USE_DMALLOC=1 to setup dmalloc
USE_DMALLOC=1
OPTIONS =

ifdef SQ_VIRTUAL_NODE
	OPTIONS =
endif

#DEBUG = -O0 -g3
DEBUG = $(CFLAGS)

FLAGS = $(DEBUG) -Wno-deprecated -fmessage-length=0
ifeq ($(USE_DMALLOC),1)
 FLAGS+= -DDMALLOC
endif
FLAGS+= -DUSE_MON_LOGGING
FLAGS+= -D_MPICC_H

# until assert side-effects are removed, leave this in
FLAGS+= -UNDEBUG
# Uncomment the following line to see which compiler option controls
#   a particular diagnostic (GNU compiler).
# FLAGS+= -fdiagnostics-show-option
FLAGS+= -Wall -Wextra

# Flags for use in compiling Seabed trace modules
SBFLAGS = -Wall -Wextra -pedantic -Werror  -Wno-long-long $(DBG_FLGS) $(OPTIM_FLGS) -Xlinker --copy-dt-needed-entries
SBDIR = ../../src/seabed/src

COMMONLOGGERDIR = ../../commonLogger

INCLUDE = monitor.h msgdef.h internal.h monlogging.h clio.h mlio.h localio.h lock.h

LIBS = -L$(LIBEXPDIR) -lrt -lsblogalt

LIBS+= -lmpich

ifeq ($(USE_DMALLOC),1)
    LIBS+= -L$(DMALLOCROOT) -ldmalloc
endif
ifeq ($(USE_FORK_SUSPEND_RESUME),1)
    FLAGS+= -DUSE_FORK_SUSPEND_RESUME
endif
ifeq ($(USE_SEQUENCE_NUM),1)
    FLAGS+= -DUSE_SEQUENCE_NUM
endif
ifeq ($(SQ_BUILD_TYPE), debug)
    FLAGS+= -DMON_DEBUG
endif

INCLUDES = -I$(INCEXPDIR)

ifeq ($(USE_DMALLOC),1)
    INCLUDES+= -I$(DMALLOCROOT)
endif
INCLUDES+= -I../../inc
INCLUDES+= -I../../commonLogger
INCLUDES+= -I$(LOG4CXX_INC_DIR) 
INCLUDES+= -I$(LOG4CXX_INC_DIR)/log4cxx

LIBS+=  -lsqlite3
LIBS+=  -llog4cxx
LIBS+=  -ldl

ifeq ($(USE_TESTPOINTS),1)
   FLAGS+= -DUSE_TESTPOINTS
endif
ifeq ($(USE_SONAR),1)
   FLAGS+= -DUSE_SONAR
endif

MONITORSRC = monitor.cxx
MONITORSRC += cluster.cxx
MONITORSRC += open.cxx
MONITORSRC += process.cxx
MONITORSRC += notice.cxx
MONITORSRC += config.cxx
MONITORSRC += monlogging.cxx
MONITORSRC += tmsync.cxx
MONITORSRC += device.cxx
MONITORSRC += monprof.cxx
MONITORSRC += montrace.cxx
MONITORSRC += monsonar.cxx
MONITORSRC += redirector.cxx
MONITORSRC += lock.cxx
MONITORSRC += clusterconf.cxx
MONITORSRC += lnodeconfig.cxx
MONITORSRC += pnodeconfig.cxx
MONITORSRC += lnode.cxx
MONITORSRC += pnode.cxx
MONITORSRC += intprocess.cxx
MONITORSRC += healthcheck.cxx
MONITORSRC += commaccept.cxx
ifeq ($(USE_DMALLOC),1)
    MONITORSRC += $(OUTDIR)/dm.cxx
endif
MONITORSRC += reqworker.cxx
MONITORSRC += reqqueue.cxx
MONITORSRC += reqattstartup.cxx
MONITORSRC += reqdump.cxx
MONITORSRC += reqevent.cxx
MONITORSRC += reqexit.cxx
MONITORSRC += reqget.cxx
MONITORSRC += reqkill.cxx
MONITORSRC += reqmonstats.cxx
MONITORSRC += reqmount.cxx
MONITORSRC += reqnewproc.cxx
MONITORSRC += reqnodedown.cxx
MONITORSRC += reqnodeinfo.cxx
MONITORSRC += reqnodename.cxx
MONITORSRC += reqnodeup.cxx
MONITORSRC += reqnotify.cxx
MONITORSRC += reqopen.cxx
MONITORSRC += reqprocinfo.cxx
MONITORSRC += reqset.cxx
MONITORSRC += reqshutdown.cxx
MONITORSRC += reqstartup.cxx
MONITORSRC += reqtmleader.cxx
MONITORSRC += reqtmready.cxx
MONITORSRC += reqtmseqnum.cxx
MONITORSRC += reqtmsync.cxx
MONITORSRC += reqzoneinfo.cxx
MONITORSRC += replicate.cxx
MONITORSRC += gentrap.cxx
MONITORSRC += monrobsem.cxx
MONITOROBJS = $(OUTDIR)/versmon.o
MONITOROBJS += $(OUTDIR)/monitor.o
MONITOROBJS += $(OUTDIR)/cluster.o
MONITOROBJS += $(OUTDIR)/open.o
MONITOROBJS += $(OUTDIR)/process.o
MONITOROBJS += $(OUTDIR)/notice.o
MONITOROBJS += $(OUTDIR)/props.o
MONITOROBJS += $(OUTDIR)/config.o
MONITOROBJS += $(OUTDIR)/monlogging.o
MONITOROBJS += $(OUTDIR)/tmsync.o
MONITOROBJS += $(OUTDIR)/device.o
MONITOROBJS += $(OUTDIR)/mlio.o
MONITOROBJS += $(OUTDIR)/monprof.o
MONITOROBJS += $(OUTDIR)/montrace.o
MONITOROBJS += $(OUTDIR)/monsonar.o
MONITOROBJS += $(OUTDIR)/threadtls.o
MONITOROBJS += $(OUTDIR)/trace.o
MONITOROBJS += $(OUTDIR)/otrace.o
MONITOROBJS += $(OUTDIR)/redirector.o
MONITOROBJS += $(OUTDIR)/lock.o
MONITOROBJS += $(OUTDIR)/clusterconf.o
MONITOROBJS += $(OUTDIR)/lnodeconfig.o
MONITOROBJS += $(OUTDIR)/pnodeconfig.o
MONITOROBJS += $(OUTDIR)/lnode.o
MONITOROBJS += $(OUTDIR)/pnode.o
MONITOROBJS += $(OUTDIR)/intprocess.o
MONITOROBJS += $(OUTDIR)/healthcheck.o
MONITOROBJS += $(OUTDIR)/commaccept.o
ifeq ($(USE_DMALLOC),1)
    MONITOROBJS += $(OUTDIR)/dm.o
endif
MONITOROBJS += $(OUTDIR)/reqworker.o
MONITOROBJS += $(OUTDIR)/reqqueue.o
MONITOROBJS += $(OUTDIR)/reqattstartup.o
MONITOROBJS += $(OUTDIR)/reqdump.o
MONITOROBJS += $(OUTDIR)/reqevent.o
MONITOROBJS += $(OUTDIR)/reqexit.o
MONITOROBJS += $(OUTDIR)/reqget.o
MONITOROBJS += $(OUTDIR)/reqkill.o
MONITOROBJS += $(OUTDIR)/reqmonstats.o
MONITOROBJS += $(OUTDIR)/reqmount.o
MONITOROBJS += $(OUTDIR)/reqnewproc.o
MONITOROBJS += $(OUTDIR)/reqnodedown.o
MONITOROBJS += $(OUTDIR)/reqnodeinfo.o
MONITOROBJS += $(OUTDIR)/reqnodename.o
MONITOROBJS += $(OUTDIR)/reqpnodeinfo.o
MONITOROBJS += $(OUTDIR)/reqnodeup.o
MONITOROBJS += $(OUTDIR)/reqnotify.o
MONITOROBJS += $(OUTDIR)/reqopen.o
MONITOROBJS += $(OUTDIR)/reqprocinfo.o
MONITOROBJS += $(OUTDIR)/reqset.o
MONITOROBJS += $(OUTDIR)/reqshutdown.o
MONITOROBJS += $(OUTDIR)/reqstartup.o
MONITOROBJS += $(OUTDIR)/reqtmleader.o
MONITOROBJS += $(OUTDIR)/reqtmready.o
MONITOROBJS += $(OUTDIR)/reqtmseqnum.o
MONITOROBJS += $(OUTDIR)/reqtmsync.o
MONITOROBJS += $(OUTDIR)/reqzoneinfo.o
ifeq ($(USE_FORK_SUSPEND_RESUME),1)
    MONITOROBJS += $(OUTDIR)/monrs.o
endif
MONITOROBJS += $(OUTDIR)/replicate.o
MONITOROBJS += $(OUTDIR)/gentrap.o
MONITOROBJS += $(OUTDIR)/monrobsem.o
MONITOROBJS += $(OUTDIR)/CommonLogger.o

SHELLOBJS = $(OUTDIR)/shell.o
SHELLOBJS += $(OUTDIR)/monclio.o
SHELLOBJS += $(OUTDIR)/props.o
SHELLOBJS += $(OUTDIR)/versshell.o
SHELLOBJS += $(OUTDIR)/clusterconf.o
SHELLOBJS += $(OUTDIR)/lnodeconfig.o
SHELLOBJS += $(OUTDIR)/pnodeconfig.o
SHELLOBJS += $(OUTDIR)/cmsh.o
SHELLOBJS += $(OUTDIR)/phnode.o
SHELLOBJS += $(OUTDIR)/system.o
SHELLOBJS += $(OUTDIR)/token.o
SHELLOBJS += $(OUTDIR)/lock.o

WATCHDOGOBJS = $(OUTDIR)/watchdog.o
WATCHDOGOBJS += $(OUTDIR)/monclio.o
WATCHDOGOBJS += $(OUTDIR)/lock.o
WATCHDOGOBJS += $(OUTDIR)/monlogging.o
WATCHDOGOBJS += $(OUTDIR)/props.o
WATCHDOGOBJS += $(OUTDIR)/sdtimer.o
WATCHDOGOBJS += $(OUTDIR)/wdtimer.o
WATCHDOGOBJS += $(OUTDIR)/pkillall.o
WATCHDOGOBJS += $(OUTDIR)/procmon.o
WATCHDOGOBJS += $(OUTDIR)/system.o
WATCHDOGOBJS += $(OUTDIR)/token.o
WATCHDOGOBJS += $(OUTDIR)/verswatch.o
WATCHDOGOBJS += $(OUTDIR)/gentrap.o
WATCHDOGOBJS += $(OUTDIR)/CommonLogger.o

PSTARTDOBJS = $(OUTDIR)/pstartd.o
PSTARTDOBJS += $(OUTDIR)/monclio.o
PSTARTDOBJS += $(OUTDIR)/lock.o
PSTARTDOBJS += $(OUTDIR)/monlogging.o
PSTARTDOBJS += $(OUTDIR)/verspstartd.o
PSTARTDOBJS += $(OUTDIR)/CommonLogger.o

MEMLOGOBJS = $(OUTDIR)/monmemlog.o
MEMLOGOBJS += $(OUTDIR)/versmemlog.o

CLIENTOBJS = $(OUTDIR)/monclio.o
CLIENTOBJS += $(OUTDIR)/props.o
CLIENTOBJS += $(OUTDIR)/clusterconf.o
CLIENTOBJS += $(OUTDIR)/lnodeconfig.o
CLIENTOBJS += $(OUTDIR)/pnodeconfig.o

MONWDTOBJS = $(OUTDIR)/monwdt.o
MONWDTOBJS += $(OUTDIR)/versmwdt.o

MSG_LOG_OBJS = $(OUTDIR)/monlogging.o

TRACE_LOG_OBJS = $(OUTDIR)/trace.o
TRACE_LOG_OBJS += $(OUTDIR)/otrace.o
TRACE_LOG_OBJS += $(OUTDIR)/threadtls.o

RTSIGBLOCK_SRC = rtsigblock.cxx
RTSIGBLOCK_OBJS = $(OUTDIR)/rtsigblock.o

ALLOBJS = $(MONITOROBJS)
ALLOBJS += $(SHELLOBJS)
ALLOBJS += $(CLIENTOBJS)
ALLOBJS += $(WATCHDOGOBJS)
ALLOBJS += $(PSTARTDOBJS)
ALLOBJS += $(MONWDTOBJS)
ALLOBJS += $(MSG_LOG_OBJS)
ALLOBJS += $(TRACE_LOG_OBJS)
ALLOBJS += $(CONFOBJS)
ALLOBJS += $(MEMLOGOBJS)
ALLOBJS += $(RTSIGBLOCK_OBJS)

PGMS = $(BINEXPDIR)/monitor $(BINEXPDIR)/shell $(BINEXPDIR)/sqwatchdog $(BINEXPDIR)/monmemlog
PGMS +=  $(BINEXPDIR)/pstartd
PGMS +=  $(LIBEXPDIR)/libseabasesig.so

TEST_PGMS = $(OUTDIR)/client
TEST_PGMS += $(OUTDIR)/client2
TEST_PGMS += $(OUTDIR)/montim
TEST_PGMS += $(OUTDIR)/nsclient
TEST_PGMS += $(OUTDIR)/server
TEST_PGMS += $(OUTDIR)/nsserver
TEST_PGMS += $(OUTDIR)/getseq
TEST_PGMS += $(OUTDIR)/notify
TEST_PGMS += $(OUTDIR)/testtm
TEST_PGMS += $(OUTDIR)/pingpong2
TEST_PGMS += $(OUTDIR)/testspx
TEST_PGMS += $(OUTDIR)/testconf

#	    $(OUTDIR)/attach \
#	    $(OUTDIR)/testenv \

# Compile rules for building sources

#$(OUTDIR)/%:%.cxx $(INCLUDE)
#	@echo 'Building target: $@'
#	@echo 'Invoking: C++ Compile & Linker'
#	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt $(OUTDIR)/clio.o $(OUTDIR)/props.o $<
#	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt $(OUTDIR)/clio.o $(OUTDIR)/props.o $<
#	@echo 'Finished building target: $@'
#	@echo ' '

$(BINEXPDIR)/%:%.cxx $(INCLUDE) $(CLIENTOBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compile & Linker'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt  $(CLIENTOBJS) $(TRACE_LOG_OBJS) $<
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt  $(CLIENTOBJS) $(TRACE_LOG_OBJS) $<
	@echo 'Finished building target: $@'
	@echo ' '

$(OUTDIR)/%:%.cxx $(INCLUDE) $(CLIENTOBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compile & Linker'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt  $(CLIENTOBJS) $(TRACE_LOG_OBJS) $<
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt  $(CLIENTOBJS) $(TRACE_LOG_OBJS) $<
	@echo 'Finished building target: $@'
	@echo ' '

$(OUTDIR)/%.o:%.cxx
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compiler'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@if [ -d "$(OUTDIR)" ]; then x=1; else mkdir -p "$(OUTDIR)"; fi
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@echo 'Finished building target: $@'
	@echo ' '

$(OUTDIR)/%.o:%.cc
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compiler'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@if [ -d "$(OUTDIR)" ]; then x=1; else mkdir -p "$(OUTDIR)"; fi
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@echo 'Finished building target: $@'
	@echo ' '

# For compiling Seabed trace modules
$(OUTDIR)/%.o:$(SBDIR)/%.cpp $(INCLUDE)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compiler'
	@echo $(CXX) $(SBFLAGS) -I$(INCEXPDIR) -I$(SBDIR) -c -o $@ $<
	@$(CXX) $(SBFLAGS) -I../../export/include -I../../src/seabed/src -c -o $@ $<
	@echo 'Finished building target: $@'
	@echo ' '

# For compiling common logger module
$(OUTDIR)/%.o:$(COMMONLOGGERDIR)/%.cpp $(INCLUDE)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compiler'
	@echo $(CXX) $(FLAGS) $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@$(CXX) $(FLAGS) $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@echo 'Finished building target: $@'
	@echo ' '

# Targets
# Uncomment the following target to disable parallel make
#.NOTPARALLEL: $(PGMS)

all: $(PGMS) example

montim:
	@$(MAKE) $(OUTDIR)/montim

example : $(TEST_PGMS)

$(OUTDIR)/client : client.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

$(OUTDIR)/nsclient : nsclient.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

$(OUTDIR)/nsclient : nsclient.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

$(OUTDIR)/server : server.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

$(OUTDIR)/nsserver : nsserver.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

$(OUTDIR)/getseq : getseq.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

$(OUTDIR)/notify : notify.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

$(OUTDIR)/testtm : testtm.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

#$(OUTDIR)/attach : attach.cxx $(OUTDIR)/clio.o

#$(OUTDIR)/testenv : testenv.cxx

$(OUTDIR)/pingpong2 : pingpong2.cxx  $(CLIENTOBJS) $(TRACE_LOG_OBJS)

CONFOBJS = $(OUTDIR)/testconf.o \
	$(OUTDIR)/clusterconf.o \
	$(OUTDIR)/lnodeconfig.o \
	$(OUTDIR)/pnodeconfig.o

$(OUTDIR)/testconf : $(CONFOBJS) $(TRACE_LOG_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Linker'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt  $^
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) $(INCLUDES) -o $@ -lrt  $^
	@echo 'Finished building target: $@'
	@echo ' '

$(BINEXPDIR)/shell : $(SHELLOBJS) $(TRACE_LOG_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Linker'
	@echo $(CXX) -lreadline -lcurses $(LNK_FLGS) -o$@ -lrt $^
	@$(CXX) -lreadline -lcurses $(LNK_FLGS) -o$@ -lrt $^
	@echo 'Finished building target: $@'
	@echo ' '

$(BINEXPDIR)/sqwatchdog : $(WATCHDOGOBJS) $(CLIENTOBJS) $(TRACE_LOG_OBJS) $(MSG_LOG_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compile & Linker'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) -pthread $(OPTIONS) $(INCLUDES) -o $@ $(LIBS) $^
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) -pthread $(INCLUDES) -o $@ $(LIBS) $^
	@echo 'Finished building target: $@'
	@echo ' '

$(BINEXPDIR)/pstartd: $(PSTARTDOBJS) $(OUTDIR)/monclio.o $(TRACE_LOG_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compile & Linker'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) -pthread $(OPTIONS) $(INCLUDES) -o $@ $(LIBS) $^
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(OPTIONS) -pthread $(INCLUDES) -o $@ $(LIBS) $^
	@echo 'Finished building target: $@'
	@echo ' '

$(BINEXPDIR)/monmemlog : $(MEMLOGOBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compile & Linker'
	@echo $(CXX) $(FLAGS) -pthread $(OPTIONS) $(INCLUDES) -o $@ $(LIBS) $^
	@$(CXX) $(FLAGS) $(OPTIONS) -pthread $(INCLUDES) -o $@ $(LIBS) $^
	@echo 'Finished building target: $@'
	@echo ' '

$(OUTDIR)/montim : montim.cxx
	@$(CXX) $(CDEPFLAGS) $(FLAGS) $(INCLUDES) -o $@ $<

$(BINEXPDIR)/monitor: $(MONITOROBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Linker'
	@echo $(CXX) $(LNK_FLGS) -o$@ $(MONITOROBJS) $(LIBS) -lz
	@$(CXX) $(LNK_FLGS) -o$@ $(MONITOROBJS) $(LIBS) -lz
	@echo 'Finished building target: $@'
	@echo ' '

$(LIBEXPDIR)/libseabasesig.so: $(RTSIGBLOCK_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Linker'
	@echo $(CXX) $(LNK_FLGS) -shared -o $@ $(RTSIGBLOCK_OBJS)
	@$(CXX) $(LNK_FLGS) -shared -o $@ $(RTSIGBLOCK_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

setup:
	@# do nothing

clean:
	-rm -f $(PGMS) $(TEST_PGMS) *.gcda *.gcno *.gcov vproc.h
	-rm -f $(ALLOBJS)
	-rm -f $(CDEP_DIR)/*.dep
	-rm -f $(LIBEXPDIR)/libseabasesig.so
	@echo ' '

cleanall: clean
	-rm -rf $(BUILD_PLAT)

cleaner: clean
	-rm -f *~

install:
	@echo 'Installing programs in $(INSTALL_DIR)'
	-cp $(PGMS) $(INSTALL_DIR)
	@echo 'Finished installing programs'
	@echo ' '

uninstall:
	@echo 'Uninstalling programs from $(INSTALL_DIR)'
	-cd $(INSTALL_DIR);rm -f $(PGMS) $(TEST_PGMS)
	@echo 'Finished uninstalling programs'
	@echo ' '

rebuild: clean all
	-@echo ' '

$(RTSIGBLOCK_OBJS):$(RTSIGBLOCK_SRC)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Compiler'
	@echo $(CXX) $(CDEPFLAGS) $(FLAGS) -fPIC $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@$(CXX) $(CDEPFLAGS) $(FLAGS) -fPIC $(OPTIONS) $(INCLUDES) -c -o $@ $<
	@echo 'Finished building target: $@'
	@echo ' '


-include $(CDEP_DIR)/*.dep
_dummy := $(shell if [ ! -d $(CDEP_DIR) ]; then mkdir -p $(CDEP_DIR); fi)
