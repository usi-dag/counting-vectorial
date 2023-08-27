package jvbench.somier;

import org.openjdk.jmh.annotations.*;

import jvbench.JMHBenchmarkConfig;

import java.util.concurrent.TimeUnit;

public class SomierBenchmark {



    @State(Scope.Thread)
    public static class MyState {

        int ntSteps = Integer.parseInt(System.getProperty("steps", "10"));
        int n = Integer.parseInt(System.getProperty("n", "128"));

        @Setup(Level.Trial)
        public void setup() {
            Somier.init(ntSteps, n);
        }
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5, jvmArgsAppend = {"-XX:-UseSuperWord"})
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void serial(MyState state, JMHBenchmarkConfig plugins) {
        Somier.scalar();
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void autoVec(MyState state, JMHBenchmarkConfig plugins) {
        Somier.scalar();
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5, jvmArgsAppend = {"-XX:-UseSuperWord"})
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void explicitVec(MyState state, JMHBenchmarkConfig plugins) {
        Somier.vector();
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void fullVec(MyState state, JMHBenchmarkConfig plugins) {
        Somier.vector();
    }
}
