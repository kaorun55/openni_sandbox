#ifndef Thread_H_INCLUDE
#define Thread_H_INCLUDE

#include <XnOS.h>
#include <map>
#include <memory>
#include <boost/utility.hpp>

namespace std {
	using namespace std::tr1;
}

// xn extend
namespace xne {
	class Thread : public boost::noncopyable
	{
	public:

		// コンストラクタ
		Thread();

		/**
		* コンストラクタ
		* @param pr コールバックさせたいオブジェクト(関数ポインタ、関数オブジェクト等)
		* @note
		*  prは int pr( Thread* thread ) の形で宣言すること関数名は何でもよい
		*/
		template< typename Pred >
		Thread(Pred pr)
			: pr_( new ThreadProc< Pred >( pr ) )
			, thread_()
		{
			Create( lpThreadAttributes ,dwStackSize );
		}

		/**
		* コンストラクタ
		* @param fn コールバックさせたいメンバ関数ポインタ
		* @param p コールバックさせたいメンバ関数ポインタを持つオブジェクトへのポインタ
		* @note
		*  fnは int T::fn( Thread* thread ) の形で宣言することメンバ関数名は何でもよい
		*/
		template< class T >
		Thread( int (T::*fn)(xne::Thread* thread ), T* p)
			: pr_( CreateThread( std::bind1st( std::mem_fun1( fn ), p ) ) )
			, thread_()
		{
			Create();
		}

		// デストラクタ
		~Thread();

		/**
		* スレッドの作成
		*
		* @param pr コールバックさせたいオブジェクト(関数ポインタ、関数オブジェクト等)
		* @param lpThreadAttributes セキュリティ記述子
		* @param dwStackSize スレッドのスタックサイズ
		*/
		template<typename Pred>
		void Create(Pred pr)
		{
			pr_ = ThreadLocalEntry(CreateThread(pr));

			Create();
		}

		/**
		* スレッドの作成
		*
		* @param fn コールバックさせたいメンバ関数ポインタ
		* @param p コールバックさせたいメンバ関数ポインタを持つオブジェクトへのポインタ
		* @param lpThreadAttributes セキュリティ記述子
		* @param dwStackSize スレッドのスタックサイズ
		*/
		template< class T >
		void Create( int (T::*fn)( xne::Thread* thread ), T* p, LPSECURITY_ATTRIBUTES lpThreadAttributes = 0, DWORD dwStackSize = 0 )
		{
			pr_ = ThreadLocalEntry(CreateThread(std::bind1st(std::mem_fun1(fn), p)));

			Create();
		}

		// 終了を待つ
		void Wait(XnUInt32 nMilliseconds = XN_WAIT_INFINITE);

		// スレッドIDを取得する
		static XN_THREAD_ID GetCurrentID();

	private:

		// スレッドの作成
		void Create();

		// 共通のスレッドのエントリポイント
		static XN_THREAD_PROC ThreadEntry(XN_THREAD_PARAM);

		// 固有のスレッドのエントリポイント
		int Run();

	private:

		/**
		* @brief スレッド関数インターフェイス
		* @note
		*  個別のスレッドエントリポイントのベースインターフェイス
		*  を作ることで、template< typename Pred > class ThreadProc
		*  にどんなテンプレート引数が与えられてもThreadProcBaseで処理
		*  できるようになる。
		*/
		class ThreadProcBase
		{
		public:

			/// デストラクタ
			virtual ~ThreadProcBase(){};

			/// エントリポイント
			virtual int Run(Thread* thread) = 0;
		};

		/// スレッド関数
		template< typename Pred >
		class ThreadProc : public ThreadProcBase
		{
		public:

			/**
			* コンストラクタ
			* @param pr コールバックさせたいオブジェクト(関数ポインタ、関数オブジェクト等)
			*/
			ThreadProc(Pred pr) : pr_(pr){}

			/**
			* デストラクタ
			*/
			~ThreadProc(){};

			/**
			* スレッドのエントリポイント
			* @param thread コールバックしたスレッドオブジェクトへのポインタ
			* @return スレッドの戻り値
			*/
			int Run(Thread* thread){ return pr_(thread); }

		private:

			Pred     pr_;
		};

		/**
		* スレッド関数オブジェクト生成関数
		* @param pr コールバックさせたいオブジェクト(関数ポインタ、関数オブジェクト等)
		* @return スレッド関数オブジェクト
		*/
		template<typename Pred>
		static ThreadProc<Pred>* CreateThread(Pred pr)
		{
			return new ThreadProc<Pred>(pr);
		}

	private:

		typedef std::shared_ptr<ThreadProcBase>   ThreadLocalEntry;
		typedef std::map<XN_THREAD_ID, Thread*>   ThreadList;

		XN_THREAD_HANDLE   thread_;    ///< スレッドハンドル
		ThreadLocalEntry   pr_;        ///< エントリポイント
		static ThreadList  threads_;   ///< スレッド一覧
	};
}

#endif // #ifndef Thread_H_INCLUDE
