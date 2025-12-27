import json
import argparse
import os
import shutil
import time

def generate_trajectory(total_requests: int, output_dir: str, clear_existing: bool = True, delay: float = 0):
    if clear_existing and os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    
    os.makedirs(output_dir, exist_ok=True)
    
    c_real = -0.8
    c_imag = 0.156
    
    step_real = 0.0015
    step_imag = 0.0008
    
    for i in range(total_requests):
        request = {
            "frame_id": i,
            "c_real": round(c_real, 6),
            "c_imag": round(c_imag, 6),
            "width": 800,
            "height": 600,
            "max_iterations": 100,
            "poly_degree": 2
        }
        
        filename = f"{output_dir}/request_{i:04d}.json"
        with open(filename, "w") as f:
            json.dump(request, f)
        
        if delay > 0:
            print(f"Created {filename}, waiting {delay:.3f}s...")
            time.sleep(delay)
        
        c_real += step_real
        c_imag += step_imag
        
        if abs(c_real) > 2.0:
            step_real *= -1
        if abs(c_imag) > 2.0:
            step_imag *= -1
    
    if delay == 0:
        print(f"{total_requests} request files in '{output_dir}/'")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate Julia set parameter trajectory for Spark Streaming"
    )
    parser.add_argument(
        "--total", 
        type=int, 
        default=60, 
        help="Total number of render requests to generate"
    )
    parser.add_argument(
        "--duration", 
        type=int, 
        default=0, 
        help="Target duration in seconds (calculates delay automatically)"
    )
    parser.add_argument(
        "--output", 
        type=str, 
        default="data/requests",
        help="Output directory for JSON files"
    )
    parser.add_argument(
        "--delay", 
        type=float, 
        default=0, 
        help="Manual delay in seconds between file generation"
    )
    parser.add_argument(
        "--keep-existing",
        action="store_true",
        help="Don't clear existing files in output directory"
    )
    
    args = parser.parse_args()
    
    final_delay = args.delay
    if args.duration > 0:
        final_delay = args.duration / args.total
        
    generate_trajectory(args.total, args.output, clear_existing=not args.keep_existing, delay=final_delay)
