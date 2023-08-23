package jvbench.axpy;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class AxpyPatternBenchmark {

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

    @State(Scope.Benchmark)
    public static class SocketPlugin {
        String serverName = "127.0.0.1"; // localhost
        int port = 1234;

        Socket clientSocket;
        PrintWriter out;
        BufferedReader in;

        AtomicInteger iteration = new AtomicInteger(0);

        String path;
        String benchmarkName; 

        public static final String ANSI_RESET = "\u001B[0m";
        public static final String ANSI_RED = "\u001B[31m";

        public SocketPlugin() {

            try {
                clientSocket = new Socket(serverName, port);
                out = new PrintWriter(clientSocket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            } catch (Exception e) {
                e.printStackTrace();
            }

            path = new File("").getAbsolutePath();
            benchmarkName = new File(path).getName();

        }

        // This function is called before a benchmark iteration starts
        @Setup(Level.Iteration)
        public void afterOperationSetUp() {

            System.out.println(ANSI_RED + "afterOperationSetUp" + ANSI_RESET);

            // SOCKET
            try {

                out.println("A" + " ~ " + benchmarkName); // Let the tool know which method is communicating and
                                                                // the iteration number

                // To make sure we do not continue before the file has been created
                // System.out.println("Waiting for server's response");
                String response = in.readLine();
                // System.out.println("Server response: " + response);
            } catch (Exception e) {
                e.printStackTrace();
            }

            // System.out.println(ANSI_RED + "afterOperationSetUp: " + benchmark + " " +
            // opIndex + " " + isLastOp + ANSI_RESET);

        }

        // This function is called once the benchmark iteration is over
        @TearDown(Level.Iteration)
        public void beforeOperationTearDown() {

            System.out.println(ANSI_RED + "beforeOperationTearDown" + ANSI_RESET);

            // SOCKET
            try {

                out.println("B" + " ~ " + benchmarkName); // Let the tool know which method is communicating and
                                                                // the iteration number

                // To make sure we do not continue before the file has been created
                // System.out.println("Waiting for server's response");
                String response = in.readLine();
                // System.out.println("Server response: " + response);
            } catch (Exception e) {
                e.printStackTrace();
            }

            // System.out.println(ANSI_RED + "beforeOperationTearDown: " + benchmark + " " +
            // opIndex + " " + durationNanos + ANSI_RESET);

        }
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    public void fmaScalar(MyState state, Blackhole blackhole, SocketPlugin socket) {
        Axpy.scalarFMA(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void indexInRange(MyState state, Blackhole blackhole, SocketPlugin socket) {
        Axpy.vectorIndexInRange(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void broadcastExternal(MyState state, Blackhole blackhole, SocketPlugin socket) {
        Axpy.vectorBroadcastExternal(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }

    @Benchmark
    @OutputTimeUnit(TimeUnit.SECONDS)
    @BenchmarkMode(Mode.SingleShotTime)
    @Fork(value = 5)
    @Warmup(iterations = 10)
    @Measurement(iterations = 10)
    public void fma(MyState state, Blackhole blackhole, SocketPlugin socket) {
        Axpy.vectorFMA(1.0, state.dx, state.dy, state.size);
        blackhole.consume(state.dx);
        blackhole.consume(state.dy);
    }
}
