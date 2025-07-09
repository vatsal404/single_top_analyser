import json
import gzip

def load_json_file(filename):
    if filename.endswith(".gz"):
        with gzip.open(filename, 'rt') as f:
            return json.load(f)
    else:
        with open(filename, 'r') as f:
            return json.load(f)

def list_corrections(json_data):
    if "corrections" not in json_data:
        print("Invalid JERC JSON file: missing 'corrections' field.")
        return

    print("Corrections found in JSON:\n")
    for correction in json_data["corrections"]:
        name = correction.get("name", "N/A")
        desc = correction.get("description", "No description")
        print(f"- Name: {name}")
        print(f"  Description: {desc}")
        print()

if __name__ == "__main__":
    # Replace this with your JERC JSON path
    filename = "data/JERC/2022_preEE/jet_jerc.json.gz"
    
    try:
        jerc_json = load_json_file(filename)
        list_corrections(jerc_json)
    except Exception as e:
        print(f"Error loading JSON: {e}")

