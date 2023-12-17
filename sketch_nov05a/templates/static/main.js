function readURL(input) {
    if (input.files && input.files[0]) {

        var reader = new FileReader();

        reader.onload = function (e) {
            $('.image-upload-wrap').hide();

            $('.file-upload-image').attr('src', e.target.result);
            $('.file-upload-content').show();

            $('.image-title').html(input.files[0].name);
        };

        reader.readAsDataURL(input.files[0]);

    } else {
        removeUpload();
    }
}

function removeUpload() {
    $('.file-upload-input').replaceWith($('.file-upload-input').clone());
    $('.file-upload-content').hide();
    $('.image-upload-wrap').show();
}
$('.image-upload-wrap').bind('dragover', function () {
    $('.image-upload-wrap').addClass('image-dropping');
});
$('.image-upload-wrap').bind('dragleave', function () {
    $('.image-upload-wrap').removeClass('image-dropping');
});

function captureImage() {
    stopCamera(); // Dừng webcam nếu đang chạy
    $('.file-upload-input').replaceWith($('.file-upload-input').clone());
    $('.file-upload-content').hide();
    $('.image-upload-wrap').show();
    $('#photoContainer').hide();
    $('#cam').show();

    // Khởi tạo webcam
    initCamera();
}
function stopCamera() {
    var video = document.getElementById('video');
    $('.file-upload').show();
    $('#cam').hide();
    if (video.srcObject) {
        var stream = video.srcObject;
        var tracks = stream.getTracks();

        tracks.forEach(function (track) {
            track.stop();
        });

        video.srcObject = null;
    }
}

// Thêm hàm để khởi tạo webcam
function initCamera() {
    $('.file-upload').hide();
    navigator.mediaDevices.getUserMedia({ video: true })
        .then(function (stream) {
            var video = document.getElementById('video');
            video.srcObject = stream;
            video.play();
        })
        .catch(function (err) {
            console.log("An error occurred: " + err);
        });
}

function captureAndPredict() {
    var video = document.getElementById('video');
    var canvas = document.createElement('canvas');
    canvas.width = video.videoWidth;
    canvas.height = video.videoHeight;
    var context = canvas.getContext('2d');

    context.drawImage(video, 0, 0, canvas.width, canvas.height);

    // Chuyển ảnh từ canvas thành file Blob
    canvas.toBlob(function (blob) {
        // Đọc lại Blob như là một ảnh
        var reader = new FileReader();
        reader.onload = function () {
            var img = new Image();
            img.onload = function () {
                $('.file-upload-image').attr('src', reader.result);
                $('.image-upload-wrap').hide();
                $('#photoContainer').show();
                $('#cam').hide();
                $('.file-upload').show();
                stopCamera();
                var canvasResized = document.createElement('canvas');
                canvasResized.width = 256;
                canvasResized.height = 256;
                var contextResized = canvasResized.getContext('2d');
                contextResized.drawImage(img, 0, 0, 256, 256);
                // Giữ lại chỉ 3 kênh từ ảnh
                var canvasRGB = document.createElement('canvas');
                canvasRGB.width = 256;
                canvasRGB.height = 256;
                var contextRGB = canvasRGB.getContext('2d');
                contextRGB.drawImage(canvasResized, 0, 0, 256, 256);
                // Chuyển ảnh đã resize và chỉ 3 kênh từ canvas thành file Blob
                canvasRGB.toBlob(function (blobResized) {
                    var formData = new FormData();
                    formData.append('file', blobResized, 'webcam_capture.jpg');

                    // Gửi dữ liệu lên server
                    sendImage(formData);
                    $('#prediction-results').show();
                }, 'image/jpeg');
            };
            img.src = reader.result;
        };
        reader.readAsDataURL(blob);
    }, 'image/jpeg');
}

function predictImage() {
    // Khởi tạo đối tượng FormData
    var formData = new FormData();

    // Thêm dữ liệu hình ảnh vào formData
    var fileInput = $('.file-upload-input')[0];
    formData.append('file', fileInput.files[0]);

    // Gửi dữ liệu lên server
    $.ajax({
        type: 'POST',
        url: '/predict',
        data: formData,
        processData: false,
        contentType: false,
        success: function (response) {
            // Cập nhật HTML để hiển thị kết quả dự đoán
            $('#predicted-class').text('Tên bệnh: ' + response.predicted_class);
            $('#confidence').text('Độ chính xác: ' + (response.confidence * 100).toFixed(2) + '%');

            // Hiển thị phần kết quả dự đoán
            $('#prediction-results').show();

            // Ghi log phản hồi đầy đủ vào console
            console.log(response);
        },
        error: function (error) {
            console.error('Error predicting image:', error);
            alert('Error predicting image. Please try again.');
        }
    });
}

function sendImage(formData) {
    $.ajax({
        type: 'POST',
        url: '/predict',
        data: formData,
        processData: false,
        contentType: false,
        success: function (response) {
            // Cập nhật HTML để hiển thị kết quả dự đoán
            $('#predicted-class').text('Tên bệnh: ' + response.predicted_class);
            $('#confidence').text('Độ chính xác: ' + (response.confidence * 100).toFixed(2) + '%');

            // Hiển thị phần kết quả dự đoán
            $('#prediction-results').show();

            // Ghi log phản hồi đầy đủ vào console
            console.log(response);
        },
        error: function (error) {
            console.error('Error predicting image:', error);
            alert('Error predicting image. Please try again.');
        }
    });
}