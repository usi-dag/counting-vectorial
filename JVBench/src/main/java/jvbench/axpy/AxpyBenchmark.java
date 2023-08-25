package jvbench.axpy;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import jvbench.JMHBenchmarkConfig;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class AxpyBenchmark {

    @State(Scope.Thread)
    public static class MyState {

        public int size;
        public double[] dy;
        public double[] dx;

        public double[] dyCopy;

        @Setup(Level.Trial)
        public void setup() {

            size = Integer.parseInt(System.getProperty("size", "70000")) * 1024;
            dx = new double[size];
            dy = new double[size];
            dyCopy = new double[size];

            for (int i = 0; i < size; i++) {
                dx[i] = 1.0;
                dy[i] = 2.0;
            }
            System.arraycopy(dy, 0, dyCopy, 0, size);
        }

        @TearDown(Level.Iteration)
        public void reset() {
            // after each iteration should reset dy
            System.arraycopy(dyCopy, 0, dy, 0, size);

        }

    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5, jvmArgsAppend = { "-XX:-UseSuperWord" })
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void serial(MyState state, Blackhole blackhole, JMHBenchmarkConfig plugins) {
        Axpy.scalar(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void autoVec(MyState state, Blackhole blackhole, JMHBenchmarkConfig plugins) {
        Axpy.scalar(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5, jvmArgsAppend = { "-XX:-UseSuperWord" })
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void explicitVec(MyState state, Blackhole blackhole, JMHBenchmarkConfig plugins) {
        Axpy.vector(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }

    @Benchmark
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void fullVec(MyState state, Blackhole blackhole, JMHBenchmarkConfig plugins) {
        Axpy.vector(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }
}
