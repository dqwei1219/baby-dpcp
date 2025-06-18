# test_client.py
import requests

class DBCPClient:
    def __init__(self, base_url="http://localhost:8080", token="your_secret_token"):
        self.base_url = base_url
        self.headers = {"Authorization": f"Bearer {token}"}
    
    def health(self):
        return requests.get(f"{self.base_url}/health").json()
    
    def query(self, sql):
        return requests.post(
            f"{self.base_url}/query",
            json={"sql": sql},
            headers=self.headers
        ).json()
    
    def execute(self, sql):
        return requests.post(
            f"{self.base_url}/execute",
            json={"sql": sql},
            headers=self.headers
        ).json()
    
# Test it
if __name__ == "__main__":
    client = DBCPClient()
    
    print("Health:", client.health())
    print("\nStats:", client.stats())
    
    # Create table if not exists
    client.execute("""
        CREATE TABLE IF NOT EXISTS test_users (
            id INT PRIMARY KEY AUTO_INCREMENT,
            name VARCHAR(100),
            email VARCHAR(100)
        )
    """)
    
    # Insert data
    result = client.execute(
        "INSERT INTO test_users (name, email) VALUES ('Test User', 'test@example.com')"
    )
    print("\nInsert result:", result)
    
    # Query data
    result = client.query("SELECT * FROM test_users")
    print("\nQuery result:", result)