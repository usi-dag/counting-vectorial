# Counting Vectorial

To instrument the benchmark "AxpyBenchmark", run the following command inside ./counting-vectorial.

```
$ pin-3.28-98749-g6643ecee5-gcc-linux/pin -t pin-3.28-98749-g6643ecee5-gcc-linux/source/tools/VectorialInstructionsCounter/obj-intel64/VectorialInstructionsCounter.so -- java -XX:+UnlockDiagnosticVMOptions -XX:+LogCompilation -cp plugin/SocketPlugin.jar:JVBench/target/JVBench-1.0.1.jar --add-modules jdk.incubator.vector -Dbenchmark.plugin=jvbench.plugin.SocketPlugin org.openjdk.jmh.Main "AxpyBenchmark"
```


## Counting Vectorial Instructions of a JVBench Benchmark
This PinTool counts the number of vectorial instructions of the JVBench benchmark being run. The results are stored in the '/results' folder.
For simplicity, both JVBench and pin are part of the repo.
In order to instrument every iteration of the JVBench benchmark, the implementation of a JVBench plugin was needed. This plugin, through the use of a Socket, tells the PinTool when an iteration starts and ends.

## Instructions being instrumented
All vectorial instructions of the type SSE/SSE2/SSE3/SSSE3/SSE4.1/SSE4.2/AVX are being instrumented. A list of all their mnemonics can be found in the file 'x86_vectorial_instructions.txt'. Instructions to instrument can be added or removed from this file.


### Using the PinTool with JVBench
To execute the Tool use:
```shell
$ pin-3.28-98749-g6643ecee5-gcc-linux/pin -t pin-3.28-98749-g6643ecee5-gcc-linux/source/tools/VectorialInstructionsCounter/obj-intel64/VectorialInstructionsCounter.so -- java -cp plugin/SocketPlugin.jar:JVBench/target/JVBench-1.0.1.jar --add-modules jdk.incubator.vector -Dbenchmark.plugin=jvbench.plugin.SocketPlugin org.openjdk.jmh.Main "<Benchmark name>"
```

For example, to instrument each iteration of the AxpyBencmark we'd run:
```shell
$ pin-3.28-98749-g6643ecee5-gcc-linux/pin -t pin-3.28-98749-g6643ecee5-gcc-linux/source/tools/VectorialInstructionsCounter/obj-intel64/VectorialInstructionsCounter.so -- java -cp plugin/SocketPlugin.jar:JVBench/target/JVBench-1.0.1.jar --add-modules jdk.incubator.vector -Dbenchmark.plugin=jvbench.plugin.SocketPlugin org.openjdk.jmh.Main "AxpyBenchmark"
```


### Modifying the PinTool
The PinTool's code can be found at '/pin-3.28-98749-g6643ecee5-gcc-linux/source/tools/VectorialInstructionsCounter/VectorialInstructionsCounter.cpp'.
To recompile the code, one needs to be in the VectorialInstructionsCounter directory and run:
```shell
$ make all target=intel64
```

### Modifying the JVBench Plugin
The Plugin's code can be found at 'JVBench/src/main/java/jvbench/plugin'.
It is implementing the interface "Plugin" situated at 'JVBench/src/main/java/jvbench/Plugin.java'.
The compilation of SocketPlugin.java is done by maven, from inside JVBench, one must run:
```shell
$ mvn clean package
```
In order to use the plugin with JVBench, it must be in '.jar' format. If one wishes to change the Plugin, it must first be recompiled (as shown above) then made into a jar. To recreate the jar, situated in 'counting-vectorial/plugin', run:
```shell
$ jar cf plugin/SocketPlugin.jar JVBench/target/classes/jvbench/plugin/SocketPlugin.class
```
