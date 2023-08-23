package jvbench;

import org.openjdk.jmh.annotations.State;

@State(Scope.Benchmark)
public static class SocketPlugin implements Plugin {
        String serverName = "127.0.0.1"; // localhost
        int port = 1234;

        Socket clientSocket;
        PrintWriter out;
        BufferedReader in;

        String path;
        String benchmarkName;

        public static final String ANSI_RESET = "\u001B[0m";
        public static final String ANSI_RED = "\u001B[31m";

        public SocketPlugin() {

            System.out.println(ANSI_RED + "SocketPlugin constructor called" + ANSI_RESET);

            try {
                clientSocket = new Socket(serverName, port);
                out = new PrintWriter(clientSocket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            } catch (Exception e) {
                e.printStackTrace();
            }



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
