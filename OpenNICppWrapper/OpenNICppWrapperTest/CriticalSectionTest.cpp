#include <gtest/gtest.h>

#include <OpenNICppWrapper/CriticalSection.h>

TEST( CriticalSection, Create )
{
	xne::CriticalSection cs;
}

TEST( CriticalSection, Lock )
{
	xne::CriticalSection cs;
	cs.Lock();

	EXPECT_TRUE( cs.IsLocked() );
}

TEST( CriticalSection, Unlock )
{
	xne::CriticalSection cs;
	cs.Lock();
	cs.Unlock();

	EXPECT_FALSE( cs.IsLocked() );
}
