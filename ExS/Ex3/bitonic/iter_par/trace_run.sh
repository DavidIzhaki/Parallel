#!/bin/bash

# Exit on error
set -e

# Build step
echo "🔨 Compiling..."
gcc -o bitonic iter_parallel.c thr_pool.c -lpthread

# Clean old session
echo "🧹 Cleaning old session (if exists)..."
lttng destroy my-session 2>/dev/null || true

# Create session
echo "📄 Creating new LTTng session..."
lttng create my-session

# Enable user-space events
echo "🎯 Enabling user-space events..."
lttng enable-event -u -a

# Start tracing
echo "▶️  Starting trace..."
lttng start

# Run the program
echo "🚀 Running bitonic..."
./bitonic

# Stop trace
echo "⏹️  Stopping trace..."
lttng stop
lttng destroy

# Show the trace
echo "📄 Showing trace output:"
babeltrace ~/lttng-traces/my-session-*
