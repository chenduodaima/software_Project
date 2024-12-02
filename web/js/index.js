//index.html分为home1、tabs、main、channels-block、bottom-banner、footer这6个盒子
//home1、tabs、main、channels-block、bottom-banner、footer这6个部分分别使用vue进行渲染
//home1中渲染最受欢迎视频（点赞最多的）
//tabs中渲染最新视频、评分最高的、观看最多的
//main中渲染编辑给的推荐视频（评论最多的）、大家都在看什么?(热度最高的)、收藏最多的
//channels-block中渲染频道列表，不需要！！！！！！！！！
//bottom-banner中渲染广告，不需要！！！！！！！！！
//footer中渲染版权信息，不需要！！！！！！！！！
//Vue3写法，不适用Vue2

const { ref, onMounted, computed, onBeforeUnmount } = Vue;

const home1 = Vue.createApp({
    setup() {
        const most_likes_videos = ref([]);
        const isLoading = ref(false);
        const hasError = ref(false);
        const searchQuery = ref('');
        const userinfo = ref({});
        const type = ref([]);
        const type_total = ref({
            "生活": 0, "搞笑": 0, "动漫": 0, "新闻": 0, "音乐": 0, "科技": 0, "教育": 0,
        });
        const videoInfoForm = ref(null);
        const timer = ref(null);

        const submitVideoInfo = () => {
            if (!videoInfoForm.value) return;

            videoInfoForm.value.addEventListener('submit', (event) => {
                event.preventDefault();
                const formData = new FormData(videoInfoForm.value);

                type.value.forEach(item => {
                    if (item.type_name === formData.get("video_type_id")) {
                        formData.set("video_type_id", item.type_id);
                    }
                });

                if (!formData.has("video") || !formData.has("image") || !formData.has("video_type_id")) {
                    alert("请上传视频文件、封面图片、选择视频类型");
                    return;
                }

                axios.post('/video', formData).then(res => {
                    alert(res.status === 200 ? "上传视频成功" : "上传视频失败");
                });
            });
        };

        const getTotal = async () => {
            const res = await axios.get('/getTotal?table_name=type');
            type.value = res.data;
            type.value.forEach(item => {
                type_total[item.type_name] = item.type_count;
            });
        };

        const pollVideos = () => {
            if (most_likes_videos.value.length === 8) {
                clearTimeout(timer.value);
                console.log(`home1数据已加载完毕，清除定时器`);
                return;
            }

            if (!isLoading.value) {
                getVideos();
            }

            timer.value = setTimeout(pollVideos, 10000);
        };

        const getVideos = () => {
            isLoading.value = true;
            $.ajax({
                url: "/video/getTopK",
                type: "get",
                headers: { Type: "likes", TopK: 8 },
                success: (res) => {
                    if (res && res.length > 0) {
                        most_likes_videos.value = res.slice(0, 8);
                        console.log(`/video/getTopK 返回数据，更新most_likes_videos`);
                    }
                },
                error: (error) => {
                    console.log(`/video/mostlikes 请求失败:`, error);
                    hasError.value = true;
                },
                complete: () => {
                    isLoading.value = false;
                }
            });
        };

        const getUserInfo = async () => {
            const res = await axios.get('/user/getUserInfo');
            userinfo.value = res.data;
        };

        const redirectToSearch = () => {
            window.location.href = `search.html?search=${searchQuery.value}`;
        };

        onMounted(() => {
            getUserInfo();
            pollVideos();
            getTotal();
            submitVideoInfo();
        });

        onBeforeUnmount(() => {
            clearTimeout(timer.value);
        });

        return {
            most_likes_videos, timer, hasError, type_total, searchQuery, redirectToSearch, userinfo, type, videoInfoForm, submitVideoInfo
        };
    },
}).mount('#home1');

const tabs = Vue.createApp({
    setup() {
        const activeTab = ref('newest');
        const newest_videos = ref([]);//最新视频,最多6个
        const most_grade_videos = ref([]);//评分最高的视频,最多6个
        const most_views_videos = ref([]);//观看最多的视频,最多6个


        const pollVideos = () => {
            getVideos();
        };

        const getVideos = () => {
            const requests = [
                { url: "/video/getTopK", target: newest_videos, headers: { 'Type': 'release_time', 'TopK': 6 } },         // 改用 ref 对象
                { url: "/video/getTopK", target: most_grade_videos, headers: { 'Type': 'grade', 'TopK': 6 } },
                { url: "/video/getTopK", target: most_views_videos, headers: { 'Type': 'views', 'TopK': 6 } },
            ];
            requests.forEach(req => {
                axios.get(req.url, { headers: req.headers })
                    .then(res => {
                        req.target.value = res.data;
                    })
                    .catch(error => {
                        console.log(`${req.url} 请求失败:`, error);
                    });
            });
        };

        const currentVideos = computed(() => {
            switch (activeTab.value) {
                case 'newest': return newest_videos.value;//最新视频
                case 'most_grade': return most_grade_videos.value;//评分最高的
                case 'most_views': return most_views_videos.value;//观看最多的
                default: return [];
            }
        });

        onMounted(() => {
            pollVideos();
        });

        return { activeTab, currentVideos };
    },
}).mount('#tabs');

const main = Vue.createApp({
    setup() {
        const daily_recommend_video = ref([]); // 每日推荐视频, 每天更新一次，最多1个
        const most_views_of_all_video = ref([]); // 全站播放量最高，最多1个
        const editor_recommend_video = ref([]); // 编辑推荐视频,最多5个
        const most_favorite_videos = ref([]); // 收藏最多的视频,最多6个
        const most_comments_videos = ref([]); // 最多评论视频,最多12个
        const commentedVideoChunks = ref([]); // 评论最多的视频，分段显示，每段6个
        const tags = ref([]); // 标签

        const pollVideos = async () => {
            await getVideos(); // 等待 getVideos 完成
            chunkCommentedVideos(); // 然后调用 chunkCommentedVideos
        };

        const getVideos = async () => {
            const requests = [
                { type: 'likes', topK: 1, target: daily_recommend_video },
                { type: 'views', topK: 1, target: most_views_of_all_video },
                { type: 'grade', topK: 5, target: editor_recommend_video },
                { type: 'favorites', topK: 6, target: most_favorite_videos },
                { type: 'comments', topK: 12, target: most_comments_videos }
            ];

            const promises = requests.map(req => {
                return new Promise((resolve, reject) => {
                    $.ajax({
                        url: "/video/getTopK",
                        type: "get",
                        headers: {
                            'Type': req.type,
                            'TopK': req.topK,
                        },
                        success: (res) => {
                            if (res && res.length > 0) {
                                req.target.value = req.topK === 1 ? [res[0]] : res;
                            }
                            resolve();
                        },
                        error: (error) => {
                            console.log(`/video/getTopK 请求失败:`, error);
                            reject(error);
                        }
                    });
                });
            });

            await Promise.all(promises); // 等待所有请求完成
        };

        // 分段显示评论最多的视频
        const chunkCommentedVideos = () => {
            while (most_comments_videos.value.length > 0) {
                commentedVideoChunks.value.push(most_comments_videos.value.splice(0, 6));
            }
        };

        const getTags = async () => {
            const res = await axios.get('/getTotal?table_name=tag');
            tags.value = res.data.slice(0, 20); // 只要前20个
        };

        onMounted(() => {
            getTags();
            pollVideos();
        });

        return {
            daily_recommend_video, most_views_of_all_video, editor_recommend_video,
            most_favorite_videos, most_comments_videos, commentedVideoChunks, tags
        };
    }
}).mount('#main');

// 头像点击跳转
document.querySelector('.avatar')?.addEventListener('click', function () {
    window.location.href = 'personal.html';
});

const updateVideoType = (type_name) => {
    document.getElementById('video_type').value = "视频类型: " + type_name;
}

