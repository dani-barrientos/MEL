#pragma once
namespace execution
{
     /**
     * @brief Hints to pass to Executor::loop
     * 
     */
    struct LoopHints
    {
        bool independentTasks = true; //<! if true, try to make each iteration independent
        virtual ~LoopHints(){} //to make it polymorphic
    };
    /**
     * @brief Hints to pass to launch
     * Concrete executor can inherit and add their custom options
     */
    struct LaunchHints
    {        
    };
}