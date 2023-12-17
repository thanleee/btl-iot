import pandas as pd
import numpy as np

# Tạo dữ liệu ngẫu nhiên cho các thông số môi trường
np.random.seed(1)

num_samples = 1000

temperature = np.random.normal(25, 2, num_samples)
humidity_air = np.random.normal(70, 5, num_samples)
humidity_soil = np.random.normal(40, 8, num_samples)

# Tạo danh sách các loại rau tương ứng với mỗi mẫu dữ liệu
vegetables = ['tomato', 'morning glory', 'cabbage']
vegetable_types = np.random.choice(vegetables, num_samples)

# Tạo DataFrame từ dữ liệu
data = pd.DataFrame({
    'temperature': temperature,
    'humidity_air': humidity_air,
    'humidity_soil': humidity_soil,
    'vegetable_type': vegetable_types
})

# In thông tin mẫu dữ liệu đầu tiên
print(data.head())