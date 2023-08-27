package jvbench.streamcluster;

import org.openjdk.jmh.annotations.*;

import jvbench.JMHBenchmarkConfig;

import java.util.concurrent.TimeUnit;

public class StreamclusterBenchmark {
    
    @State(Scope.Thread)
    public static class MyState {

        long kMin = Long.parseLong(System.getProperty("k1", "3"));
        long kMax = Long.parseLong(System.getProperty("k2", "10"));
        int dim = Integer.parseInt(System.getProperty("dim", "128"));

        int chunkSize = Integer.parseInt(System.getProperty("chunksize", "128"));
        int clusterSize = Integer.parseInt(System.getProperty("clustersize", "10"));
        String inputFileName = System.getProperty("input", "/streamcluster/input/streamcluster_128_128.input");
        String outputFileName = System.getProperty("outfile", "output.txt");
        PStream stream;
        
        
        @Setup(Level.Trial)
        public void setup() {
            stream = StreamCluster.init(inputFileName);
        }
    }
    
    
    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5, jvmArgsAppend = {"-XX:-UseSuperWord"})
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void serial(MyState state, JMHBenchmarkConfig plugins) {
        StreamCluster.streamCluster(state.stream, state.kMin, state.kMax, state.dim, state.chunkSize, state.clusterSize, state.outputFileName, false);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void autoVec(MyState state, JMHBenchmarkConfig plugins) {
        StreamCluster.streamCluster(state.stream, state.kMin, state.kMax, state.dim, state.chunkSize, state.clusterSize, state.outputFileName, false);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5, jvmArgsAppend = {"-XX:-UseSuperWord"})
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void explicitVec(MyState state, JMHBenchmarkConfig plugins) {
        StreamCluster.streamCluster(state.stream, state.kMin, state.kMax, state.dim, state.chunkSize, state.clusterSize, state.outputFileName, true);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void fullVec(MyState state, JMHBenchmarkConfig plugins) {
        StreamCluster.streamCluster(state.stream, state.kMin, state.kMax, state.dim, state.chunkSize, state.clusterSize, state.outputFileName, true);
    }
}
