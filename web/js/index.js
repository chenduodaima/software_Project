let app = new Vue({
    el: '#tabs',
    data: {
        activeTab: 'new',
        author: ["涂瑞晨", "黄昊", "李岚欣", "熊振昱", "周智琪"],
        hot_videos: [],
        max_heat_videos: [],
        new_videos: [],
        most_commented_videos: [],
        most_collected_videos: [],
        top_rated_videos: [],
        timer: null
    },
    computed: {
        currentVideos() {
            switch(this.activeTab) {
                case 'new': return this.new_videos;
                case 'hot': return this.hot_videos;
                case 'commented': return this.most_commented_videos;
                default: return [];
            }
        }
    },
    methods: {
        pollVideos() {
            this.getVideos();
            // 设置下一次轮询
            this.timer = setTimeout(this.pollVideos, 5000);
        },
        
        getVideos() {
            const requests = [
                { url: "/video/heat", target: 'hot_videos' },
                { url: "/video/maxheat", target: 'max_heat_videos' },
                { url: "/video/newest", target: 'new_videos' },
                { url: "/video/mostcommented", target: 'most_commented_videos' },
                { url: "/video/mostviewed", target: 'most_collected_videos' },
                { url: "/video/mostfavorited", target: 'most_collected_videos' },
                { url: "/video/mostliked", target: 'most_liked_videos' },
                { url: "/video/toprated", target: 'top_rated_videos' }
            ];

            requests.forEach(req => {
                $.ajax({
                    url: req.url,
                    type: "get",
                    context: this,
                    success: function(res) {
                        if (!res || res.length === 0) {
                            console.log(`${req.url} 返回空数据`);
                            return;
                        }
                        this[req.target] = res;
                    },
                    error: function(xhr, status, error) {
                        console.log(`${req.url} 请求失败:`, error);
                    }
                });
            });
        }
    },
    mounted() {
        this.pollVideos(); // 开始轮询
    },
    beforeDestroy() {
        // 组件销毁前清除定时器
        if (this.timer) {
            clearTimeout(this.timer);
        }
    }
});

// 头像点击跳转
document.querySelector('.avatar')?.addEventListener('click', function() {
    window.location.href = 'personal.html';
});