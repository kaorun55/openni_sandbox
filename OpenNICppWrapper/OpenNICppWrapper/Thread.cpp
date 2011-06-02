#include "Thread.h"

namespace xne {
	// スレッドIDとthisポインタの関連付け
	Thread::ThreadList  Thread::threads_;

	// コンストラクタ
	Thread::Thread()
		: pr_()
		, thread_()
	{
	}

	// デストラクタ
	Thread::~Thread()
	{
		Wait();
	}

	// スレッドの作成
	void Thread::Create()
	{
		XnStatus rc = ::xnOSCreateThread(&Thread::ThreadEntry, 0, &thread_);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(::xnGetStatusString(rc));
		}

		threads_[GetCurrentID()] = this;
	}

	// 終了を待つ
	void Thread::Wait(XnUInt32 nMilliseconds /*= XN_WAIT_INFINITE*/)
	{
		XnStatus rc = ::xnOSWaitForThreadExit(thread_, nMilliseconds);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(::xnGetStatusString(rc));
		}
	}

	//スレッドIDを取得する
	/*static*/ XN_THREAD_ID Thread::GetCurrentID()
	{
		XN_THREAD_ID threadId = 0;
		XnStatus rc = ::xnOSGetCurrentThreadID(&threadId);
		if (rc != XN_STATUS_OK) {
			throw std::runtime_error(::xnGetStatusString(rc));
		}

		return threadId;
	}

	// 共通のスレッドのエントリポイント
	/*static*/ XN_THREAD_PROC Thread::ThreadEntry(XN_THREAD_PARAM)
	{
		Thread* pThread = threads_[GetCurrentID()];
		XN_ASSERT( pThread != 0 );
		XN_THREAD_PROC_RETURN(pThread->Run());
	}

	// スレッドのエントリポイント
	int Thread::Run()
	{
		return pr_->Run(this);
	}
}
// EOF
