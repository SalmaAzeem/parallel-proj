import grpc

_channel = None
_stub = None

def get_stub(target: str = "127.0.0.1:50051,127.0.0.1:50052"):
    global _channel, _stub
    
    if _stub is None:
        options = [
            ('grpc.lb_policy_name', 'round_robin'),
            ('grpc.enable_retries', 1),
            ('grpc.service_config', '''{
                "methodConfig": [{
                    "name": [{}],
                    "retryPolicy": {
                        "maxAttempts": 3,
                        "initialBackoff": "0.1s",
                        "maxBackoff": "1s",
                        "backoffMultiplier": 2,
                        "retryableStatusCodes": ["UNAVAILABLE"]
                    }
                }]
            }''')
        ]
        
        _channel = grpc.insecure_channel(f"ipv4:{target}", options=options)
        
        from . import fractal_pb2_grpc
        _stub = fractal_pb2_grpc.FractalServiceStub(_channel)
    
    return _stub
