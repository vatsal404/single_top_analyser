import json

def explore_json(obj, path="root"):
    """ Recursively explore JSON structure and print all keys/paths """
    if isinstance(obj, dict):
        for key, value in obj.items():
            explore_json(value, path + f"['{key}']")
    elif isinstance(obj, list):
        print(f"{path} is a list with {len(obj)} elements")
        if len(obj) > 0 and isinstance(obj[0], dict):  # Check first element if it's a dict
            explore_json(obj[0], path + "[0]")  # Explore structure of first element

# Load JSON file
with open("bdt_model.json", "r") as f:
    bdt_model = json.load(f)

# Explore structure
explore_json(bdt_model)

