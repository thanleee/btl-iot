import uvicorn
from fastapi import FastAPI, File, UploadFile,Request,Form 
import numpy as np
from io import BytesIO
from PIL import Image
import tensorflow as tf
from fastapi.responses import HTMLResponse,JSONResponse
from fastapi.templating import Jinja2Templates
from typing import Optional
from fastapi.staticfiles import StaticFiles
import connectdb
from pydantic import BaseModel

app = FastAPI()
templates = Jinja2Templates(directory=["templates", "data"])

app.mount("/static", StaticFiles(directory="templates/static"), name="static")

# MODEL = tf.keras.models.load_model(r"G:\iot_plan\potato\model\3")
MODEL = tf.keras.models.load_model(r"C:\Users\Admin\Desktop\sketch_nov05a (4)\sketch_nov05a\model\4")
CLASS_NAMES = ['Bệnh đốm vòng (Úa sớm)', 'Bệnh úa muộn', 'Cây khỏe mạnh']

@app.get("/index/", response_class=HTMLResponse)
async def index(request: Request):
    context = {'request': request}
    return templates.TemplateResponse("predict.html", context)

@app.post("/savedata")
async def savedata(
    temperature: float = Form(...),
    humidity: float = Form(...),
    soilMoisture: float = Form(...),
):
    try:
        # Gọi hàm insertData từ module connectdb
        connectdb.insertData(temperature, humidity, soilMoisture)
        return {"message": "Dữ liệu đã được chèn thành công"}
    except Exception as e:
        print("Lỗi khi chèn dữ liệu:", str(e))
        return {"message": "Đã xảy ra lỗi khi chèn dữ liệu"}

@app.get("/", response_class=HTMLResponse)
async def index(request: Request):
    context = {'request': request}
    result_data = connectdb.dudoan()
    context['test_value'] = result_data
    return templates.TemplateResponse("index.html", context)

def read_file_as_image(data) -> np.ndarray:
    try:
        image = Image.open(BytesIO(data))
        if image.format is None:
            print("Unsupported image format.")
            return None
        target_size = (256, 256)
        image = image.resize(target_size)
        # Convert the image to a NumPy array
        image_array = np.array(image)
        return image_array
    except Exception as e:
        print(f"Error reading image: {e}")
        return None
@app.route('/savedata', methods=['GET', 'POST'])
def savedata():
    if Request.method == "POST":
        nhietdo = Request.form.get("temperature")
        doam = Request.form.get("humidity")
        doamdat = Request.form.get("soilMoisture")
        # nhietdo = "3"
        # doam = "4"
        # doamdat = "54"
        # connectdb.insertData(nhietdo , doam , doamdat)
    # return jsonify({"message": "Dữ liệu đã được lưu thành công"})
        try:
            # Gọi hàm insertData từ module connectdb
            connectdb.insertData(nhietdo, doam, doamdat)
            return "Dữ liệu đã được chèn thành công"
        except Exception as e:
            print("Lỗi khi chèn dữ liệu:", str(e))
            return "Đã xảy ra lỗi khi chèn dữ liệu"

@app.post("/predict")
async def predict(
    file: UploadFile = File(...)

):
    image = read_file_as_image(await file.read())
    img_batch = np.expand_dims(image, 0)
    prediction = MODEL.predict(img_batch)
    print("Prediction:")
    print(prediction)
    # Lấy chỉ số của lớp có xác suất cao nhất
    predicted_class_index = np.argmax(prediction[0])
    # Lấy tên của lớp từ danh sách CLASS_NAMES
    predicted_class_name = CLASS_NAMES[predicted_class_index]
    confidence = np.max(prediction[0])
    print("Predicted Class:")
    print(predicted_class_name)
    
    return {"prediction": prediction.tolist(),
             "predicted_class": predicted_class_name, 
             "class_names": CLASS_NAMES, 
             "confidence": float(confidence) }


if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=5500)
