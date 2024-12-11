const { ref, onMounted, computed, onBeforeUnmount } = Vue;

const home1 = Vue.createApp({
    setup() {
        const most_heat_videos = ref([]);
        const isLoading = ref(false);
        const hasError = ref(false);
        const searchQuery = ref('');
        const userinfo = ref({});
        const totalType = ref([]);
        const videoInfoForm = ref(null);
        const timer = ref(null);

        const getTotalType = () => {
            axios.get(`/getTotal?table_name=type`).then(response => {
                totalType.value = response.data;
                console.log(totalType.value);
            }).catch(error => {
                console.error('Error fetching total type:', error);
            });
        }

        const submitVideoInfo = () => {
            if (!videoInfoForm.value) return;
        
            let videoDuration = null; // 用于存储视频时长
        
            // 监听视频文件选择事件
            const videoInput = document.querySelector('input[name="video"]');
            videoInput.addEventListener('change', (event) => {
                const file = event.target.files[0];
                if (file) {
                    const videoElement = document.createElement('video');
                    videoElement.preload = 'metadata';
        
                    videoElement.onloadedmetadata = function() {
                        window.URL.revokeObjectURL(videoElement.src);
                        videoDuration = videoElement.duration; // 获取视频时长
                    };
        
                    videoElement.src = URL.createObjectURL(file);
                }
            });
        
            videoInfoForm.value.addEventListener('submit', (event) => {
                event.preventDefault();
                const formData = new FormData(videoInfoForm.value);
        
                totalType.value.forEach(item => {
                    if (item.type_name === formData.get("video_type_id")) {
                        formData.set("video_type_id", item.type_id);
                    }
                });
        
                if (!formData.has("video") || !formData.has("image") || !formData.has("video_type_id")) {
                    alert("请上传视频文件、封面图片、选择视频类型");
                    return;
                }
        
                if (videoDuration !== null) {
                    //将时长转化为0:0:0的形式，第一个为小时，第二个为分钟，第三个为秒
                    const hours = Math.floor(videoDuration / 3600);
                    const minutes = Math.floor((videoDuration % 3600) / 60);
                    const seconds = Math.floor(videoDuration % 60);
                    formData.set("duration", `${hours}:${minutes}:${seconds}`); // 添加视频时长到 FormData
                } else {
                    alert("无法获取视频时长，请重试");
                    return;
                }
                console.log(formData);
                console.log(formData.get("duration"));
                axios.post('/video', formData).then(res => {
                    alert(res.status === 200 ? "上传视频成功" : "上传视频失败");
                });
            });
        };

        const pollVideos = () => {
            if (most_heat_videos.value.length < 8) {
                if (!isLoading.value) {
                    getVideos();
                }
                timer.value = setTimeout(pollVideos, 3000); // 每3秒检查一次
            } else {
                clearTimeout(timer.value);
                console.log(`home1数据已加载完毕，清除定时器`);
            }
        };

        const getVideos = () => {
            isLoading.value = true;
            $.ajax({
                url: "/video/getTopK",
                type: "get",
                headers: { Type: "heat", TopK: 8 },
                success: (res) => {
                    if (res && res.length > 0) {
                        most_heat_videos.value = res.slice(0, 8);
                    }
                },
                error: (error) => {
                    console.log(`/video/mostheat 请求失败:`, error);
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
            submitVideoInfo();
            getTotalType();
        });

        onBeforeUnmount(() => {
            clearTimeout(timer.value);
        });

        return {
            most_heat_videos, timer, hasError, searchQuery, redirectToSearch, userinfo, videoInfoForm, submitVideoInfo, totalType
        };
    },
}).mount('#home1');

const tabs = Vue.createApp({
    setup() {
        const activeTab = ref('newest');
        const newest_videos = ref([]);
        const most_likes_videos = ref([]);
        const most_views_videos = ref([]);

        const pollVideos = () => {
            if (newest_videos.value.length < 6 || most_likes_videos.value.length < 6 || most_views_videos.value.length < 6) {
                getVideos();
                setTimeout(pollVideos, 3000); // 每3秒检查一次
            }
        };

        const getVideos = () => {
            const requests = [
                { url: "/video/getTopK", target: newest_videos, headers: { 'Type': 'release_time', 'TopK': 6 } },
                { url: "/video/getTopK", target: most_likes_videos, headers: { 'Type': 'likes', 'TopK': 6 } },
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
                case 'newest': return newest_videos.value;
                case 'most_likes': return most_likes_videos.value;
                case 'most_views': return most_views_videos.value;
                default: return [];
            }
        });

        onMounted(() => {
            newest_videos.value = [];
            most_likes_videos.value = [];
            most_views_videos.value = [];
            pollVideos();
        });

        return { activeTab, currentVideos };
    },
}).mount('#tabs');

const main = Vue.createApp({
    setup() {
        const most_likes_video = ref([]);
        const most_views_of_all_video = ref([]);
        const most_dislikes_video = ref([]);
        const most_favorite_videos = ref([]);
        const most_comments_videos = ref([]);
        const commentedVideoChunks = ref([]);
        const tags = ref([]);

        const pollVideos = async () => {
            await getVideos();
            chunkCommentedVideos();
            if (most_likes_video.value.length < 1 || most_views_of_all_video.value.length < 1 || most_dislikes_video.value.length < 5 || most_favorite_videos.value.length < 6 || most_comments_videos.value.length < 12) {
                setTimeout(pollVideos, 3000); // 每3秒检查一次
            }
        };

        const getVideos = async () => {
            const requests = [
                { type: 'likes', topK: 1, target: most_likes_video },
                { type: 'views', topK: 1, target: most_views_of_all_video },
                { type: 'dislikes', topK: 5, target: most_dislikes_video },
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

            await Promise.all(promises);
        };

        const chunkCommentedVideos = () => {
            while (most_comments_videos.value.length > 0) {
                commentedVideoChunks.value.push(most_comments_videos.value.splice(0, 6));
            }
        };

        const getTags = async () => {
            const res = await axios.get('/getTotal?table_name=tag');
            tags.value = res.data.slice(0, 20);
        };

        onMounted(() => {
            getTags();
            pollVideos();
        });

        return {
            most_likes_video, most_views_of_all_video, most_dislikes_video,
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