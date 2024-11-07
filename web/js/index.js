//index.html分为home1、tabs、main、channels-block、bottom-banner、footer这6个盒子
//home1、tabs、main、channels-block、bottom-banner、footer这6个部分分别使用vue进行渲染
//home1中渲染最受欢迎视频（点赞最多的）
//tabs中渲染最新视频、评分最高的、观看最多的
//main中渲染编辑给的推荐视频（评论最多的）、大家都在看什么?(热度最高的)、收藏最多的
//channels-block中渲染频道列表，不需要！！！！！！！！！
//bottom-banner中渲染广告，不需要！！！！！！！！！
//footer中渲染版权信息，不需要！！！！！！！！！
const { ref, onMounted } = Vue;

Vue.createApp({
    setup() {
        const most_liked_videos = ref([]);//点赞最多的视频,最多8个
        const timer = ref(null);
        const isLoading = ref(true);
        const hasError = ref(false);

        const pollVideos = () => {
            if (most_liked_videos.value.length === 8) {
                isLoading.value = false;
                clearTimeout(timer.value);
                return;
            }
            getVideos();
            timer.value = setTimeout(pollVideos, 5000);
        };

        const getVideos = () => {
            $.ajax({
                url: "/video/mostliked",
                type: "get",
                success: (res) => {
                    if (!res || res.length === 0) {//如果返回的数据为空，则不更新most_liked_videos
                        console.log(`/video/mostliked 返回空数据`);
                        return;
                    }
                    //如果返回的数据不为空，且most_liked_videos长度小于8，则添加数据至8个
                    while (most_liked_videos.value.length < 8 && res.length > 0) {
                        most_liked_videos.value.push(res.shift());//满了8个就停止
                    }
                    //如果返回的数据小于8，且most_liked_videos长度等于8
                    //从most_liked_videos中删除数组开头的数据，再添加返回的数据
                    if (res.length < 8 && most_liked_videos.value.length === 8) {
                        while (most_liked_videos.value.length > res.length) {
                            most_liked_videos.value.shift();
                        }
                        most_liked_videos.value.push(...res);
                    }
                    //如果返回的数据大于8，则更新most_liked_videos
                    if (res.length > 8) {
                        most_liked_videos.value = res.slice(0, 8);//只保留前8个
                    }
                },
                error: (error) => {
                    console.log(`/video/mostliked 请求失败:`, error);
                    hasError.value = true;
                    isLoading.value = false;
                }
            })
        };

        onMounted(() => {
            pollVideos();
        });

        return {
            most_liked_videos,
            timer,
            isLoading,
            hasError
        };
    },
}).mount('#home1');

Vue.createApp({
    setup() {
        const isLoading = ref(true);
        const activeTab = ref('new');
        const newest_videos = ref([]);//最新视频
        const top_rated_videos = ref([]);//评分最高的视频
        const most_viewed_videos = ref([]);//观看最多的视频
        const timer = ref(null);


        const pollVideos = () => {
            getVideos();
            // 设置下一次轮询
            timer.value = setTimeout(pollVideos, 5000);
        };

        const getVideos = () => {
            const requests = [
                { url: "/video/newest", target: 'newest_videos' },//最新视频
                { url: "/video/mostcommented", target: 'most_commented_videos' },//评论最多的
                { url: "/video/mostviewed", target: 'most_viewed_videos' },//观看最多的
            ];

            requests.forEach(req => {
                $.ajax({
                    url: req.url,
                    type: "get",
                    success: function(res) {
                        if (!res || res.length === 0) {
                            console.log(`${req.url} 返回空数据`);
                            return;
                        }
                        this[req.target] = res;
                    },
                    error: function(xhr, status, error) {
                        isLoading.value = false;
                        console.log(`${req.url} 请求失败:`, error);
                    }
                });
            });
        }

        const currentVideos = computed(() => {
            switch(activeTab.value) {
                case 'new': return newest_videos.value;//最新视频
                case 'top': return top_rated_videos.value;//评分最高的
                case 'viewed': return most_viewed_videos.value;//观看最多的
                default: return [];
            }
        });

        onMounted(() => {
            pollVideos(); // 开始轮询
        });

        onBeforeUnmount(() => {// 组件销毁前清除定时器
            if (timer.value) {
                clearTimeout(timer.value);
            }
        });

        return {
            activeTab,
            currentVideos,
            isLoading,
        };
    }, 
}).mount('#tabs');

// // 头像点击跳转
// document.querySelector('.avatar')?.addEventListener('click', function() {
//     window.location.href = 'personal.html';
// });

// let main = Vue.createApp({
//     el: '#main',
//     data: {
//         isLoading: true,
//         top_rated_videos: [],//评分最高的视频
//         most_liked_videos: [],//点赞最多的视频
//         timer: null
//     },
//     methods: {
//         pollVideos() {
//             this.getVideos();
//             // 设置下一次轮询
//             this.timer = setTimeout(this.pollVideos, 5000);
//         },

//         getVideos() {
//             this.isLoading = true;  // 开始加载
//             $.ajax({
//                 url: "/video/toprated", 
//                 type: "get", 
//                 context: this, 
//                 success: function(res) {
//                     if (!res || res.length === 0) {
//                         console.log(`/video/toprated 返回空数据`);
//                         this.top_rated_videos = [];
//                         return;
//                     }
//                     this.top_rated_videos = res;
//                 },
//                 error: function(xhr, status, error) {
//                     console.log(`/video/toprated 请求失败:`, error);
//                     this.top_rated_videos = [];
//                 },
//                 complete: () => {
//                     this.isLoading = false;  // 完成加载
//                 }
//             });
//             $.ajax({
//                 url: "/video/mostliked", 
//                 type: "get", 
//                 context: this, 
//                 success: function(res) {
//                     if (!res || res.length === 0) {
//                         console.log(`/video/mostliked 返回空数据`);
//                         return;
//                     }
//                     this.most_liked_videos = res;
//                 },
//                 error: function(xhr, status, error) {
//                     console.log(`/video/mostliked 请求失败:`, error);
//                 }
//             });
//         }
//     },
//     mounted() {
//         this.pollVideos(); // 开始轮询
//     },
//     beforeDestroy() {
//         if (this.timer) {
//             clearTimeout(this.timer);
//         }
//     }
// });