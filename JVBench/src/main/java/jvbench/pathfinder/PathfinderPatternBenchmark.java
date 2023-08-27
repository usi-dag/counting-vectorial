package jvbench.pathfinder;

import jvbench.JMHBenchmarkConfig;
import jvbench.pathfinderConditionInsideLoop.PathFinder;
import org.openjdk.jmh.annotations.*;

import java.util.concurrent.TimeUnit;

public class PathfinderPatternBenchmark {



    @State(Scope.Thread)
    public static class ConditionInsideLoopState {
        @Setup(Level.Trial)
        public void setup() {
            PathFinder.init(System.getProperty("input", "/pathfinder/input/pathfinder_5000_5000.input"));
        }
    }

    @State(Scope.Thread)
    public static class IndexInRangeState {
        @Setup(Level.Trial)
        public void setup() {
            jvbench.pathfinderIndexInRange.PathFinder.init(System.getProperty("input", "/pathfinder/input/pathfinder_5000_5000.input"));
        }
    }


    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void conditionInsideLoop(ConditionInsideLoopState state, JMHBenchmarkConfig plugins) {
        PathFinder.vector();
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    // split
    public void indexInRange(IndexInRangeState state, JMHBenchmarkConfig plugins) {
        jvbench.pathfinderIndexInRange.PathFinder.vector();
    }
}
