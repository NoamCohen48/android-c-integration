package com.example.composebuttondemo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalInspectionMode
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.example.composebuttondemo.ui.theme.ComposeButtonDemoTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            ComposeButtonDemoTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    CounterScreen(modifier = Modifier.padding(innerPadding))
                }
            }
        }
    }
}

@Composable
fun CounterScreen(modifier: Modifier = Modifier) {
    // The number we probe with the native library; advances on each tap.
    var n by remember { mutableIntStateOf(1) }

    // The native .so isn't available to Android Studio's preview renderer, so
    // skip the JNI calls when inspecting and show placeholders instead.
    val inspecting = LocalInspectionMode.current

    // Both values below are computed in C++ (mathutils) via JNI/SWIG.
    val isPrime = if (inspecting) false else MathUtils.isPrime(n.toLong())
    val primes = if (inspecting) emptyList() else MathUtils.primesUpTo(n.toLong())

    Column(
        modifier = modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Text(
            text = "n = $n",
            style = MaterialTheme.typography.headlineMedium
        )
        Text(
            text = "is_prime(n) = $isPrime",
            style = MaterialTheme.typography.bodyLarge,
            modifier = Modifier.padding(top = 8.dp)
        )
        Text(
            text = "primes ≤ n = ${primes.joinToString(", ").ifEmpty { "—" }}",
            style = MaterialTheme.typography.bodyLarge,
            modifier = Modifier.padding(top = 4.dp)
        )
        Button(
            onClick = { n++ },
            modifier = Modifier.padding(top = 24.dp)
        ) {
            Text("Next number (run C++)")
        }
    }
}

@Preview(showBackground = true)
@Composable
fun CounterScreenPreview() {
    ComposeButtonDemoTheme {
        CounterScreen()
    }
}
