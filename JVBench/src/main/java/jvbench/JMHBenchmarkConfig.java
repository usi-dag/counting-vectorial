package jvbench;

import java.util.ArrayList;
import java.util.List;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.TearDown;

@State(Scope.Benchmark)
public class JMHBenchmarkConfig {

	public JMHBenchmarkConfig() {
		final String pluginClasses = System.getProperty("benchmark.plugin");
		if (pluginClasses != null) {
			try {
				for (final String pluginClass : pluginClasses.split(";")) {
					if (!pluginClass.isEmpty()) {
						plugins.add((Plugin) Class.forName(pluginClass).newInstance());
					}
				}
				// plugins.add(new SocketPlugin());
			} catch (Exception ex) {
				throw new RuntimeException(ex);
			}
		}
	}

	private final List<Plugin> plugins = new ArrayList<>();
	private int iteration = 0;

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
}
