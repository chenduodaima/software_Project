let vue = require('Vue');

//index.html实现获取视频列表渲染链接列表

//1.创建vue实例
let app = new Vue({
    el: '#app',
    data: {
        author : ["涂瑞晨", "黄昊", "李岚欣", "熊振昱", "周智琪"],
        videos : []
    },
    methods: {
        getVideos : function(){
            $.ajax({
                url : "/video",
                type : "get",
                context : this,
                success : function(res, status, xhr){
                    this.videos = res;
                }
            });
        }   
    },
});

app.getVideos();



