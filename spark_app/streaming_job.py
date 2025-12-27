from pyspark.sql import SparkSession
from pyspark.sql.types import StructType, StructField, DoubleType, IntegerType
import time
import csv
import os

METRICS_FILE = "data/spark_metrics.csv"

request_schema = StructType([
    StructField("frame_id", IntegerType(), nullable=False),
    StructField("c_real", DoubleType(), nullable=False),
    StructField("c_imag", DoubleType(), nullable=False),
    StructField("width", IntegerType(), nullable=True),
    StructField("height", IntegerType(), nullable=True),
    StructField("max_iterations", IntegerType(), nullable=True),
    StructField("poly_degree", IntegerType(), nullable=True),
])

def init_metrics_file():
    os.makedirs(os.path.dirname(METRICS_FILE), exist_ok=True)
    with open(METRICS_FILE, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["sent_time", "end_time", "frame_id", "worker", "latency_ms", "calc_time_ms", "success"])

def process_micro_batch(batch_df, batch_id: int):
    def render_partition(partition_iterator):
        from spark_app.grpc_client import get_stub
        from spark_app import fractal_pb2
        import grpc
        
        stub = get_stub()
        
        for row in partition_iterator:
            start_time = time.time()
            worker_addr = "unknown"
            calc_time_ms = 0.0
            
            try:
                request = fractal_pb2.JuliaRequest(
                    c_real=row.c_real,
                    c_imag=row.c_imag,
                    width=row.width or 800,
                    height=row.height or 600,
                    max_iterations=row.max_iterations or 100,
                    poly_degree=row.poly_degree or 2,
                    x_min=-2.0,
                    x_max=2.0,
                    y_min=-2.0,
                    y_max=2.0,
                )
                
                response = stub.CalculateJulia(request)
                calc_time_ms = response.calculation_time_ms
                worker_addr = response.server_id or "unknown"
                success = 1
                
            except Exception as e:
                print(f"Frame {row.frame_id}: {e}")
                success = 0
            
            latency_ms = (time.time() - start_time) * 1000
            end_time = time.time()
            yield (start_time, end_time, row.frame_id, worker_addr, latency_ms, calc_time_ms, success)
    
    results = batch_df.rdd.mapPartitions(render_partition).collect()
    
    with open(METRICS_FILE, "a", newline="") as f:
        writer = csv.writer(f)
        for r in results:
            writer.writerow(r)
            status = "OK" if r[6] else "FAIL"
            print(f"{batch_id} Frame {r[2]:3d} -> {r[3]}: {r[4]:.1f}ms (calc: {r[5]:.2f}ms) {status}")

def main(total_requests: int, duration: int):
    trigger_seconds = duration / total_requests
    trigger_interval = f"{trigger_seconds} seconds"
    
    print("=" * 60)
    print("SPARK STRUCTURED STREAMING - FRACTAL RENDERER")
    print("=" * 60)
    print(f"Total Requests:    {total_requests}")
    print(f"Duration:          {duration}s")
    print(f"Trigger Interval:  {trigger_interval}")
    print(f"Metrics Output:    {METRICS_FILE}")
    print("=" * 60)
    
    init_metrics_file()
    
    spark = SparkSession.builder.appName("FractalStreaming").getOrCreate()
    spark.sparkContext.setLogLevel("WARN")
    
    stream = spark.readStream.schema(request_schema).json("data/requests/")
    
    query = stream.writeStream.foreachBatch(process_micro_batch).trigger(processingTime=trigger_interval).start()
    
    print("Waiting for data in data/requests/...")
    print("Press Ctrl+C to stop")
    
    query.awaitTermination()

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Spark Structured Streaming job for fractal rendering via gRPC"
    )
    parser.add_argument(
        "--total",
        type=int,
        default=60,
        help="Total requests expected"
    )
    parser.add_argument(
        "--duration",
        type=int,
        default=60,
        help="Target duration in seconds for the entire stream"
    )
    
    args = parser.parse_args()
    main(args.total, args.duration)
