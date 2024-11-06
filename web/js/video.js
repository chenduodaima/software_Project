
$(".nav .dropdown").hover(function () {
    $(this).find(".dropdown-toggle").dropdown("toggle");
});


let app = new Vue({
    el: '#myapp',
    data: {
        author: "Zhangwenchao",
        video: {}
    },
    methods: {
        get_param: function (name) {
            return decodeURIComponent((new RegExp('[?|&]' + name + '=' + '([^&;]+?)(&|#|;|$)').exec(location.href) || [, ""])[1].replace(/\+/g, '%20')) || null
        },
        get_video: function () {
            var id = this.get_param("id");
            $.ajax({
                url: "/video/" + id,
                type: "get",
                context: this,
                success: function (result, status, xhr) {
                    this.video = result;
                }
            })
        },
        update_video: function () {
            $.ajax({
                url: "/video/" + this.video.id,
                type: "put",
                data: JSON.stringify(this.video),
                context: this,
                success: function (result, status, xhr) {
                    alert("修改视频信息成功");
                    window.location.reload();
                }
            })
        },
        delete_video: function () {
            $.ajax({
                url: "/video/" + this.video.id,
                type: "delete",
                context: this,
                success: function (result, status, xhr) {
                    alert("删除视频成功");
                    window.location.href = "/index.html";
                }
            })
        }
    }
});
app.get_video();
