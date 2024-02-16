import requests
from flask import Flask, request, jsonify
app = Flask(__name__)

# Default route
@app.route("/")
def hello():
    return "Welcome to NutriScanner"

@app.route("/api/nutrition-info", methods=['GET'])
def get_nutritional_info():
    '''

    Given a barcode, return nutritional information about the product from the Open Food Facts API
    Arguments:     barcode
    Error cases:   (1) Missing argument
                   (2) No barcode or invalid barcode 
                   (3) Request to Open Food Facts failed, unable to extract nutritional information
    Success case:  product is found in Open Food Facts API, correct nutritional incormation is returned 

    '''
    endpoint = "/api/nutrition-info/"
    response = {}
    status = 503
    url = "https://world.openfoodfacts.net/api/v2/product/"

    try:
        barcode = request.args.get("barcode", None)
        if not barcode:
            response["MESSAGE"] = f"ERROR: {endpoint} barcode argument is missing"
            status = 400
        else:
            response = {}
            try:
                url = url + barcode
                res = requests.get(url)
                res_json = res.json()
                if(res_json['status'] == 0):
                    response["MESSAGE"] = f"EXCEPTION: {endpoint} no code or invalid code {barcode}"
                    print(response["MESSAGE"])
                    status = 500
                else:
                    response["MESSAGE"] = f"SUCCESS: {endpoint} information for {barcode} retrieved successfully"
                    print(response["MESSAGE"])
                    response["calories"] = res_json["product"]["nutriments"]["energy-kcal_serving"]
                    response["fat"] = res_json["product"]["nutriments"]["fat_serving"]
                    response["carbs"] = res_json["product"]["nutriments"]["carbohydrates_serving"]
                    response["protein"] = res_json["product"]["nutriments"]["proteins_serving"]
                    response["serving_quantity"] = res_json["product"]["serving_quantity"]        
                    status = 200
            except Exception as e:
                response["MESSAGE"] = f"EXCEPTION: {endpoint} {e}"
                print(response["MESSAGE"])
                status = 500
    except Exception as e:
        response["MESSAGE"] = f"EXCEPTION: /info/ {e}"
        print(response["MESSAGE"])
        status = 500
    return jsonify(response, status)

if __name__ == "__main__":
	app.run(host='0.0.0.0', port=80, debug=True)
