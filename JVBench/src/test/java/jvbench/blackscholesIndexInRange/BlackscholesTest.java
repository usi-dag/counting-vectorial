package jvbench.blackscholesIndexInRange;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class BlackscholesTest {


    @BeforeEach
    public void setUp() {
        Blackscholes.init("/blackscholes/input/in_4K.input");
    }

    @Test
    public void indexInRangePattern() {
        Blackscholes.vector();
        float [] prices = Blackscholes.getPrices();
        OptionData[] data = Blackscholes.getData();

        for (int i=0; i < data.length; i++) {
            float priceDelta = data[i].getDGrefval() - prices[i];
            assertTrue(Math.abs(priceDelta) < 1e-4, "Error on " + i + ". Computed=" + prices[i] + ", Ref=" + data[i].getDGrefval() + ", Delta=" + priceDelta + ",");
        }
    }
}