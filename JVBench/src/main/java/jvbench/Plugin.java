package jvbench;

public interface Plugin {

    default void beforeTrialSetup() {}
    default void afterTrialTearDown() {}
    default void beforeIterationSetup(int iteration) {}
    default void afterIterationTearDown(int iteration) {}
    default void beforeInvocationSetup() {}
    default void afterInvocationTearDown() {}

}