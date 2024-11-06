let app = new Vue({
    el: '#tabs',
    data: {
        isLoading: true,
        activeTab: 'new',
        author: ["涂瑞晨", "黄昊", "李岚欣", "熊振昱", "周智琪"],
        hot_videos: [],//热度视频
        max_heat_videos: [],//热度最高的视频
        newest_videos: [],//最新视频
        most_commented_videos: [],//评论最多的视频
        most_collected_videos: [],//收藏最多的视频
        timer: null
    },
    computed: {
        currentVideos() {
            switch(this.activeTab) {
                case 'new': return this.newest_videos;
                case 'hot': return this.max_heat_videos;
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
                { url: "/video/newest", target: 'newest_videos' },
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

let main = new Vue({
    el: '#main',
    data: {
        isLoading: true,
        top_rated_videos: [],//评分最高的视频
        most_liked_videos: [],//点赞最多的视频
        timer: null
    },
    methods: {
        pollVideos() {
            this.getVideos();
            // 设置下一次轮询
            this.timer = setTimeout(this.pollVideos, 5000);
        },

        getVideos() {
            this.isLoading = true;  // 开始加载
            $.ajax({
                url: "/video/toprated", 
                type: "get", 
                context: this, 
                success: function(res) {
                    if (!res || res.length === 0) {
                        console.log(`/video/toprated 返回空数据`);
                        this.top_rated_videos = [];
                        return;
                    }
                    this.top_rated_videos = res;
                },
                error: function(xhr, status, error) {
                    console.log(`/video/toprated 请求失败:`, error);
                    this.top_rated_videos = [];
                },
                complete: () => {
                    this.isLoading = false;  // 完成加载
                }
            });
            $.ajax({
                url: "/video/mostliked", 
                type: "get", 
                context: this, 
                success: function(res) {
                    if (!res || res.length === 0) {
                        console.log(`/video/mostliked 返回空数据`);
                        return;
                    }
                    this.most_liked_videos = res;
                },
                error: function(xhr, status, error) {
                    console.log(`/video/mostliked 请求失败:`, error);
                }
            });
        }
    },
    mounted() {
        this.pollVideos(); // 开始轮询
    },
    beforeDestroy() {
        if (this.timer) {
            clearTimeout(this.timer);
        }
    }
});