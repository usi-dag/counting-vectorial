package jvbench;

import java.util.ArrayList;
import java.util.List;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.TearDown;
import org.openjdk.jmh.infra.BenchmarkParams;
import org.openjdk.jmh.infra.IterationParams;
import org.openjdk.jmh.runner.IterationType;

@State(Scope.Benchmark)
public class JMHBenchmarkConfig {

	public JMHBenchmarkConfig() {
		final String pluginClasses = System.getProperty("benchmark.plugin");
		if (pluginClasses != null) {
			try {
				for (final String pluginClass : pluginClasses.split(";")) {
					if (!pluginClass.isEmpty()) {
						plugins.add((Plugin) Class.forName(pluginClass).getDeclaredConstructor().newInstance());
					}
				}
			} catch (Exception ex) {
				throw new RuntimeException(ex);
			}
		}
	}

	private final List<Plugin> plugins = new ArrayList<>();

	@Setup(Level.Iteration)
	public void beforeIterationSetup(BenchmarkParams benchmarkParams, IterationParams iterationParams) {
		for (Plugin plugin : plugins) {
			if(iterationParams.getType() == IterationType.MEASUREMENT)
				plugin.beforeIterationSetup(benchmarkParams.getBenchmark());
		}
	}

	@TearDown(Level.Iteration)
	public void afterIterationTearDown(BenchmarkParams benchmarkParams, IterationParams iterationParams) {
		for (Plugin plugin : plugins) {
			if(iterationParams.getType() == IterationType.MEASUREMENT)
				plugin.afterIterationTearDown(benchmarkParams.getBenchmark());
		}
	}
}
