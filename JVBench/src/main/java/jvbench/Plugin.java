package jvbench;

public interface Plugin {

    default void beforeTrialSetup() {}
    default void afterTrialTearDown() {}
    default void beforeIterationSetup(String benchmarkName) {}
    default void afterIterationTearDown(String benchmarkName) {}
    default void beforeInvocationSetup() {}
    default void afterInvocationTearDown() {}

}