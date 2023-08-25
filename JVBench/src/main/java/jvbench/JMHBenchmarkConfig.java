package jvbench;

import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;
import java.util.function.DoubleUnaryOperator;
import java.util.function.IntUnaryOperator;
import java.util.function.LongUnaryOperator;
import java.util.function.UnaryOperator;
import java.util.stream.Collectors;

// import org.json.simple.JSONArray;
// import org.json.simple.JSONObject;
// import org.json.simple.parser.JSONParser;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.TearDown;

@State(Scope.Benchmark)
public class JMHBenchmarkConfig {

	// private final Node state;

	// private void invalidArgument(final String message) {
	// 	throw new IllegalArgumentException("JSON format error: " + message + ".");
	// }

	// private <T> T getValue(final JSONObject config, final String identifier, final Class<T> type) {
	// 	final Object jsonValue = config.get(identifier);

	// 	if (jsonValue == null) {
	// 		invalidArgument("config does not have a \"" + identifier + "\" field");
	// 	}

	// 	if (!type.isInstance(jsonValue)) {
	// 		invalidArgument("\"" + identifier + "\" field is not of type \"" + type.getName() + "\"");
	// 	}

	// 	return (T) jsonValue;
	// }

	// private <T> T getValueOrNull(final JSONObject config, final String identifier, final Class<T> type) {
	// 	try {
	// 		return getValue(config, identifier, type);
	// 	} catch (Exception ex) {
	// 		return null;
	// 	}
	// }

	// private Node parseState(final JSONObject config) {
	// 	// TODO : proper exception handling
	// 	try {
	// 		final Long repetitions = getValueOrNull(config, "repetitions", Long.class);
	// 		final boolean fork = getValue(config, "fork", Boolean.class);

	// 		final JSONArray jsonChildren = getValue(config, "children", JSONArray.class);
	// 		final List<? extends Node> children =
	// 				(List<? extends Node>) jsonChildren.stream()
	// 								 .map(child -> {
	// 									 if (!(child instanceof JSONObject)) {
	// 										 invalidArgument("children array does not contain a valid element.");
	// 									 }
										 
	// 									 return parseState((JSONObject) child);
	// 								 })
	// 								 .collect(Collectors.toList());

	// 		if (repetitions != null) {
	// 			return new GroupNode(repetitions, fork, children);
	// 		}

	// 		final long size = getValue(config, "size", Long.class);

	// 		if (size < 1) {
	// 			invalidArgument("size field must be greater than zero.");
	// 		}

	// 		final String costClass = getValue(config, "cost", String.class);
	// 		final boolean parallel = getValue(config, "parallel", Boolean.class);
	// 		final String workloadClass = getValue(config, "workload", String.class);

	// 		final Object cost = Class.forName((String) costClass).newInstance();
	// 		final Workload<?> workload = (Workload<?>) Class.forName((String) workloadClass).newInstance();

	// 		if (workload instanceof IntWorkload) {
	// 			return new IntStreamNode(
	// 				size, (IntUnaryOperator) cost, parallel, fork, (IntWorkload) workload, children
	// 			);
	// 		} else if (workload instanceof LongWorkload) {
	// 			return new LongStreamNode(
	// 				size, (LongUnaryOperator) cost, parallel, fork, (LongWorkload) workload, children
	// 			);
	// 		} else if (workload instanceof DoubleWorkload) {
	// 			return new DoubleStreamNode(
	// 				size, (DoubleUnaryOperator) cost, parallel, fork, (DoubleWorkload) workload, children
	// 			);
	// 		} /* else if (workload instanceof ReferenceWorkload) { } */

	// 		return new ReferenceStreamNode(
	// 			size, (UnaryOperator) cost, parallel, fork, (ReferenceWorkload) workload, children
	// 		);
	// 	} catch (Exception ex) {
	// 		throw new RuntimeException(ex);
	// 	}
	// }

	public JMHBenchmarkConfig() {
		//
		// ************** STATE **************
		//
		// final String jsonConfigFile = System.getProperty("benchmark.configFile");
		// final String jsonConfigString = System.getProperty("benchmark.config");

        // final JSONParser jsonParser = new JSONParser();

		// if (jsonConfigFile == null && jsonConfigString == null) {
		// 	throw new RuntimeException("No json configuration provided. Please use either -Dbenchmark.configFile=<path> or -Dbenchmark.config=<json>");
		// }

		// JSONObject config;

		// try {
		// 	if (jsonConfigFile != null) {
		// 		config = (JSONObject) jsonParser.parse(new FileReader(jsonConfigFile));
		// 	} else {
		// 		config = (JSONObject) jsonParser.parse(jsonConfigString);
		// 	}
		// } catch (Exception ex) {
		// 	throw new RuntimeException(ex);
		// }

		// state = parseState(config);

		//
		// ************** PLUGINS **************
		//
		final String pluginClasses = System.getProperty("benchmark.plugin");
		if (pluginClasses != null) {
			try {
				for (final String pluginClass : pluginClasses.split(";")) {
					if (!pluginClass.isEmpty()) {
						plugins.add((Plugin) Class.forName(pluginClass).getDeclaredConstructor().newInstance());
					}
				}
				// plugins.add(new SocketPlugin());
			} catch (Exception ex) {
				throw new RuntimeException(ex);
			}
		}
	}

	// public Node state() {
	// 	return state;
	// }
	
	//
	// ************** LIFECYCLE **************
	//

	private final List<Plugin> plugins = new ArrayList<>();
	private int iteration = 0;

	// @Setup(Level.Trial)
	// public void beforeTrialSetup() {
	// 	for (Plugin plugin : plugins) {
	// 		plugin.beforeTrialSetup();
	// 	}
	// }

	// @TearDown(Level.Trial)
	// public void afterTrialTearDown() {
	// 	for (Plugin plugin : plugins) {
	// 		plugin.afterTrialTearDown();
	// 	}
	// }

	@Setup(Level.Iteration)
	public void beforeIterationSetup() {
		iteration++;
		for (Plugin plugin : plugins) {
			plugin.beforeIterationSetup(iteration);
		}
	}

	@TearDown(Level.Iteration)
	public void afterIterationTearDown() {
		for (Plugin plugin : plugins) {
			plugin.afterIterationTearDown(iteration);
		}
	}

	// @Setup(Level.Invocation)
	// public void beforeInvocationSetup() {
	// 	for (Plugin plugin : plugins) {
	// 		plugin.beforeInvocationSetup();
	// 	}
	// }

	// @TearDown(Level.Invocation)
	// public void afterInvocationTearDown() {
	// 	for (Plugin plugin : plugins) {
	// 		plugin.afterInvocationTearDown();
	// 	}
	// }

}
