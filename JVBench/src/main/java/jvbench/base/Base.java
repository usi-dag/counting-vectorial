package JVBench.src.main.java.jvbench.base;

// Socket
import java.io.*;
import java.net.*;

@State(Scope.Thread)
public class Base {

    String serverName = "127.0.0.1"; // localhost
    int port = 1234;

    Socket clientSocket;
    PrintWriter out;
    BufferedReader in;

    public Base() {
        try {
            clientSocket = new Socket(serverName, port);
            out = new PrintWriter(clientSocket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // This function is called before a benchmark iteration starts
    @Setup(Level.Iteration) // FIXME: Is this called before each iteration or before the benchmark setsup?
    public void baseSetup() {
        // SOCKET
        try {

            out.println("A" + opIndex + " ~ " + benchmark); // Let the tool know which method is communicating and the
                                                            // iteration number

            // To make sure we do not continue before the file has been created
            // System.out.println("Waiting for server's response");
            String response = in.readLine();
            // System.out.println("Server response: " + response);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @TearDown(Level.Iteration)
    public void baseTearDown() {    // FIXME: Is this called after each iteration or after the benchmark setsup?  
        // SOCKET
        try {

            out.println("B" + opIndex + " ~ " + benchmark); // Let the tool know which method is communicating and the
                                                            // iteration number

            // To make sure we do not continue before the file has been created
            // System.out.println("Waiting for server's response");
            String response = in.readLine();
            // System.out.println("Server response: " + response);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

public class SpecificBenchmark1 extends BenchmarkBase {
    // Your benchmark methods here
}

public class SpecificBenchmark2 extends BenchmarkBase {
    // Your benchmark methods here
}
