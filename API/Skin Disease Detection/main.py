import shutil
import numpy as np
import tensorflow as tf
from fastapi import FastAPI
from tensorflow import keras
from keras.models import load_model
from fastapi import UploadFile, File

app = FastAPI()

dog_skin_disease_model = load_model("Dog-skin-disease-model.h5")
dog_skin_disease_labels = ['Bacterial Dermatosis', 'Fungal Infections', 'Hypersensitivity Allergic']

def predict_image_model(model, image_path, class_names, target_size):
    loaded_image = keras.preprocessing.image.load_img(image_path, target_size=target_size)
    img_array = keras.preprocessing.image.img_to_array(loaded_image)
    img_array = np.expand_dims(img_array, axis=0)
    predictions = model.predict(img_array)
    score = tf.nn.softmax(predictions[0])
    return dog_skin_disease_labels[tf.argmax(score)]

@app.post("/dog_skin_disease-predict")
async def dog_skin_disease_predict(file: UploadFile = File(...)):
    with open("tempfile.jpg", "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)
    
    # Process the file with your model
    return predict_image_model(dog_skin_disease_model, "tempfile.jpg", dog_skin_disease_labels, (120, 120))
