# counting-vectorial

To instrument the benchmark "AxpyBenchmark", run the following command inside ./counting-vectorial.

```
$ pin-3.28-98749-g6643ecee5-gcc-linux/pin -t pin-3.28-98749-g6643ecee5-gcc-linux/source/tools/MyPinTool/obj-intel64/MyPinTool.so -s 1 -- java -cp plugin/SocketPlugin.jar:JVBench/target/JVBench-1.0.1.jar --add-modules jdk.incubator.vector -Dbenchmark.plugin=jvbench.plugin.SocketPlugin org.openjdk.jmh.Main "AxpyBenchmark"
```