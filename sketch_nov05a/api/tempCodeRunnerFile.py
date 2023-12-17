import uvicorn
from fastapi import FastAPI,File, UploadFile
import numpy as np
from io import BytesIO
from PIL import Image
app = FastAPI()


@app.get("/ping")
async def ping():
    return "Hello I'm Thanh"

def read_file_as_image(data) -> np.ndarray:
    image = np.array(Image.open(BytesIO(data)))
    return image

@app.post("/predict")
async def predict(
    file:  UploadFile = File(...)

):
    image = read_file_as_image(await file.read())

    return