//
// Created by unite on 17.05.2024.
//

#include <utils/job_system.h>
#include "gtest/gtest.h"

TEST(CoreLib, FuncJob)
{
    core::FuncJob job{ [](){} };

    EXPECT_FALSE(job.HasStarted());
    EXPECT_FALSE(job.IsDone());
    EXPECT_TRUE(job.ShouldStart());

    job.Execute();

    EXPECT_TRUE(job.IsDone());
    EXPECT_TRUE(job.HasStarted());
    EXPECT_TRUE(job.ShouldStart());

    job.Reset();

    EXPECT_FALSE(job.HasStarted());
    EXPECT_FALSE(job.IsDone());
    EXPECT_TRUE(job.ShouldStart());
}

TEST(CoreLib, DependencyJob)
{
    auto parentJob = std::make_shared<core::FuncJob>( [](){} );
    core::FuncDependentJob job{parentJob, [](){} };

    EXPECT_FALSE(job.ShouldStart());

    parentJob->Execute();

    EXPECT_TRUE(job.ShouldStart());
    job.Execute();
    EXPECT_TRUE(job.IsDone());
}

TEST(CoreLib, DependenciesJob)
{
    auto parentJob1 = std::make_shared<core::FuncJob>( [](){} );
    auto parentJob2 = std::make_shared<core::FuncJob>( [](){} );
    auto parentJob3 = std::make_shared<core::FuncJob>( [](){} );
    core::FuncDependenciesJob job{{parentJob1, parentJob2}, [](){} };

    EXPECT_FALSE(job.ShouldStart());

    parentJob1->Execute();
    EXPECT_FALSE(job.ShouldStart());
    parentJob2->Execute();
    EXPECT_TRUE(job.ShouldStart());

    job.AddDependency(parentJob3);
    EXPECT_FALSE(job.ShouldStart());
    parentJob3->Execute();
    EXPECT_TRUE(job.ShouldStart());


    job.Execute();
    EXPECT_TRUE(job.IsDone());
}
