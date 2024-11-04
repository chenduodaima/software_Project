//video.html实现通过参数id获取单个视频属性

let app = new Vue({
    el : "#app",
    data : {
        video : {},
        id_Name : "id"
    },
    methods : {
        getParam : function(name){
            const urlParams = new RegExp("[\\?&]" + name + "=([^&#])*").exec(window.location.herf);
            if (urlParams == null){
                return null;
            }
            return decodeURIComponent(urlParams[1]);
        },
        getVideo : function(){
            const id = this.getParam(this.id_Name);
            $.ajax({
                url : "/video/" + id,
                type : "get",
                context : this,
                success : function(res, status, xhr){
                    this.video = res;
                },
            });
        }
    }
});