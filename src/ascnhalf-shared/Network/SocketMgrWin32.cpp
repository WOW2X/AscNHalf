/*
  * AscNHalf MMORPG Server
  * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
  * Copyright (C) 2010 AscNHalf Team <http://ascnhalf.scymex.info/>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU Affero General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU Affero General Public License for more details.
  *
  * You should have received a copy of the GNU Affero General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  */

#include "Network.h"
#ifdef CONFIG_USE_IOCP

#include "../CrashHandler.h"

initialiseSingleton(SocketMgr);
SocketMgr::SocketMgr()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);

}

SocketMgr::~SocketMgr()
{

}

void SocketMgr::SpawnWorkerThreads()
{
	SetThreadName("Worker Spawner");

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	uint8 threadcnt = uint8(si.dwNumberOfProcessors*2);
	DEBUG_LOG("IOCP","Spawning %u worker threads.", threadcnt);
	for(long x = 0; x < threadcnt; ++x)
		ThreadPool.ExecuteTask(new SocketWorkerThread());
}

bool SocketWorkerThread::run()
{
	SetThreadName("Socket Worker");
	HANDLE cp = sSocketMgr.GetCompletionPort();
	DWORD len;
	Socket * s;
	OverlappedStruct * ov;
	LPOVERLAPPED ol_ptr;

	while(true)
	{
#ifndef _WIN64
		if(!GetQueuedCompletionStatus(cp, &len, (LPDWORD)&s, &ol_ptr, 10000))
#else
		if(!GetQueuedCompletionStatus(cp, &len, (PULONG_PTR)&s, &ol_ptr, 10000))
#endif
			continue;

		ov = CONTAINING_RECORD(ol_ptr, OverlappedStruct, m_overlap);

		if(ov->m_event == SOCKET_IO_THREAD_SHUTDOWN)
		{
			delete ov;
			return true;
		}

		if(ov->m_event < NUM_SOCKET_IO_EVENTS)
			ophandlers[ov->m_event](s, len);
	}

	return true;
}

void HandleReadComplete(Socket * s, uint32 len)
{
	//s->m_readEvent=NULL;
	if(!s->IsDeleted())
	{
		s->m_readEvent.Unmark();
		if(len)
		{
			s->GetReadBuffer().IncrementWritten(len);
			s->OnRead();
			s->SetupReadEvent();
		}
		else
			s->Delete();	  // Queue deletion.
	}
}

void HandleWriteComplete(Socket * s, uint32 len)
{
	if(!s->IsDeleted())
	{
		s->m_writeEvent.Unmark();
		s->BurstBegin();					// Lock
		s->GetWriteBuffer().Remove(len);
		if( s->GetWriteBuffer().GetContiguiousBytes() > 0 )
			s->WriteCallback();
		else
			s->DecSendLock();
		s->BurstEnd();					  // Unlock
	}
}

void HandleShutdown(Socket * s, uint32 len)
{

}

void SocketMgr::CloseAll()
{
	list<Socket*> tokill;

	socketLock.Acquire();
	for(set<Socket*>::iterator itr = _sockets.begin(); itr != _sockets.end(); ++itr)
		tokill.push_back(*itr);
	socketLock.Release();

	for(list<Socket*>::iterator itr = tokill.begin(); itr != tokill.end(); ++itr)
		(*itr)->Disconnect();

	size_t size;
	do
	{
		socketLock.Acquire();
		size = _sockets.size();
		socketLock.Release();
	}while(size);
}

void SocketMgr::ShutdownThreads()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	uint8 threadcnt = uint8(si.dwNumberOfProcessors*2);
	for(int i = 0; i < threadcnt; ++i)
	{
		OverlappedStruct * ov = NULL;
		ov = new OverlappedStruct(SOCKET_IO_THREAD_SHUTDOWN);
		PostQueuedCompletionStatus(m_completionPort, 0, (ULONG_PTR)0, &ov->m_overlap);
	}
}

#endif

