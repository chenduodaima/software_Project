const { ref, onMounted } = Vue;

// 视频信息管理模块
const videoModule = () => {
    const video = ref({});//视频信息
    const tags = ref([]);//视频标签
    const comments = ref([]);//视频评论
    const current_comment = ref('');//当前评论
    const video_user_info = ref({});//视频用户信息
    const comment_user_info = ref([]);//视频评论用户信息
    //将comments和comment_user_info合并
    const comment_user_info_map = ref([]);
    //评论分块，每个块包含5个评论
    const comment_block = ref([]);
    //当前评论块的索引
    const current_comment_block_index = ref(0);
    const similar_video = ref([]);//相似视频，要排除自己
    const user_video_relation = ref({
        is_liked: '0',
        dislike: '0',
        favorite: '0',
        grade: '0.0',
    });//用户和视频的关系

    //获取用户和视频的关系
    const get_user_video_relation = () => {
        axios.get("/video/getVideoRelation?video_id=" + video.value.video_id)
            .then((response) => {
                user_video_relation.value = response.data;
                // console.log("user_video_relation : ", user_video_relation.value);
            })
            .catch((error) => {
                console.error("获取用户和视频的关系失败", error);
            });
    };
    
    //将user_video_relation转换为字符串
    const user_video_relation_str = () => {
        return "is_liked=" + user_video_relation.value.is_liked + 
        "&dislike=" + user_video_relation.value.dislike + 
        "&favorite=" + user_video_relation.value.favorite + 
        "&grade=" + user_video_relation.value.grade;
    };

    //更新用户和视频的关系
    const update_user_video_relation = (type) => {
        if (type !== 'grade') {
            // console.log(user_video_relation.value[type])
            if (user_video_relation.value[type] === '0') {
                user_video_relation.value[type] = '1';
            } else {
                user_video_relation.value[type] = '0';
            }
            // console.log(user_video_relation.value[type])
            // console.log("user_video_relation : ", user_video_relation.value);
        }else {
            user_video_relation.value.grade = grade;
        }
        axios.post("/video/updateVideoRelation?video_id=" + video.value.video_id + "&" + user_video_relation_str())
            .then(() => {
                console.log("更新用户和视频的关系成功");
                get_video();
            })
            .catch((error) => {
                console.error("更新用户和视频的关系失败", error);
            });
    };

    // 获取视频id
    const get_param = (name) => {
        return decodeURIComponent((new RegExp('[?|&]' + name + '=' + '([^&;]+?)(&|#|;|$)').exec(location.href) || [, ""])[1].replace(/\+/g, '%20')) || null;
    };

    //获取相似视频
    const get_similar_video = () => {   
        axios.get("/video/getVideoOfType?type_id=" + video.value.video_type_id + "&number=6")
            .then((response) => {
                similar_video.value = response.data;
                //排除自己
                similar_video.value = similar_video.value.filter((item) => item.video_id !== video.value.video_id);
                // console.log("similar_video : ", similar_video.value);
            })
            .catch((error) => {
                console.error("获取相似视频失败", error);
            });
    };

    // 获取视频信息
    const get_video = () => {
        const id = get_param("id");
        axios.get("/video/" + id)
            .then((result) => {
                video.value = result.data;
                get_video_user_info();//获取视频用户信息
                get_similar_video();//获取相似视频
                // 确保在映射之前获取评论和用户信息
                return Promise.all([get_comments(), get_comment_user_info()]);//获取评论和评论用户信息
            })
            .then(() => {
                map_comment_user_info();//将评论和评论用户信息映射
                get_user_video_relation();//获取用户和视频的关系
            })
            .catch((error) => {
                console.error("获取视频信息失败", error);
            }); 
        axios.get("/tag/" + id)
            .then((data) => {
                if (data.status === 204) {
                    tags.value = [];
                } else if (data.status === 200) {
                    tags.value = data.data;
                }
            })
            .catch((error) => {
                console.error("获取标签信息失败", error);
            });
    };


    // 获取视频用户信息
    const get_video_user_info = () => {
        axios.get("/user/getVideoUserInfo?id=" + video.value.video_id)
            .then((response) => {
                video_user_info.value = response.data;
            })
            .catch((error) => {
                console.error("获取视频用户信息失败", error);
            });
    };

    // 获取评论
    const get_comments = () => {
        return axios.get("/comment/getComment?id=" + video.value.video_id)
            .then((response) => {
                comments.value = response.data;
                //console.log("comments : ", comments.value);
            })
            .catch((error) => {
                console.error("获取评论失败", error);
            });
    };

    // 发送评论
    const send_comment = () => {
        if (current_comment.value === '') {
            alert("评论不能为空");
            return;
        }
        console.log(current_comment.value);
        axios.post("/comment/sendComment?id=" + video.value.video_id, {
            comment: current_comment.value,
        })
            .then((response) => {
                if (response.status === 200) {
                    // 确保在映射之前获取评论和用户信息
                    current_comment.value = '';
                    return Promise.all([get_comments(), get_comment_user_info()]);
                } else {
                    alert("发送评论失败");
                    current_comment.value = '';
                }
            })
            .then(() => {
                map_comment_user_info();
                console.log("发送评论成功, current_comment : ", current_comment.value);
            })
            .catch((error) => {
                console.error("发送评论失败", error);
            });
    };

    //获得评论用户信息
    const get_comment_user_info = () => {
        return axios.get("/user/getCommentUserInfo?id=" + video.value.video_id)
            .then((response) => {
                comment_user_info.value = response.data;
                //console.log("comment_user_info : ", comment_user_info.value);
            })
            .catch((error) => {
                console.error("获取评论用户信息失败", error);
            });
    };

    //将comments和comment_user_info映射，通过comments的sender_uid找到comment_user_info的uid
    const map_comment_user_info = () => {
        comment_user_info_map.value = [];
        //遍历comments
        for (let i = 0; i < comments.value.length; i++) {
            //在comment_user_info中找到对应的user_info
            const user_info = comment_user_info.value.find((item) => item.uid === comments.value[i].sender_uid);
            comment_user_info_map.value.push({
                comment: comments.value[i],
                user_info: user_info,
            });
        }
        //清空comments和comment_user_info
        comments.value = [];
        comment_user_info.value = [];
        comment_block.value = [];
        //分块
        for (let i = 0; i < comment_user_info_map.value.length; i += 5) {
            comment_block.value.push(comment_user_info_map.value.slice(i, i + 5));
        }
        //清空comment_user_info_map
        comment_user_info_map.value = [];
        // console.log("comment_block : ", comment_block.value);
        //console.log("comment_user_info_map : ", comment_user_info_map.value);
    };

    //上一页
    const prevBlock = () => {
        if (current_comment_block_index.value > 0) {
            current_comment_block_index.value--;
        } else {
            alert("已经是第一页了");
        }
    };

    //下一页
    const nextBlock = () => {
        if (current_comment_block_index.value < comment_block.value.length - 1) {
            current_comment_block_index.value++;
        } else {
            alert("已经是最后一页了");
        }
    };

    return { video, tags, get_video, current_comment, 
    send_comment, video_user_info, similar_video, 
    comment_block, current_comment_block_index, prevBlock, nextBlock, user_video_relation, update_user_video_relation };
};

// 用户信息管理模块
const userModule = () => {
    const userinfo = ref({});

    // 获取登录用户信息
    const getUserInfo = async () => {
        try {
            const res = await axios.get('/user/getUserInfo');
            userinfo.value = res.data;
        } catch (error) {
            console.error("获取用户信息失败", error);
        }
    };

    return { userinfo, getUserInfo };
};

// 搜索功能模块
const userSearch = () => {
    const searchQuery = ref('');

    // 跳转搜索页面
    const redirectToSearch = () => {
        window.location.href = `/search.html?search=${searchQuery.value}`;
    };

    return { searchQuery, redirectToSearch };
};

const myapp = Vue.createApp({
    setup() {
        const { video, tags, get_video, current_comment, 
        send_comment, video_user_info, similar_video, comment_block, user_video_relation, update_user_video_relation,
        current_comment_block_index, prevBlock, nextBlock} = videoModule();
        const { userinfo, getUserInfo } = userModule();
        const { searchQuery, redirectToSearch } = userSearch();

        onMounted(async () => {
            await get_video();
            getUserInfo();
        });

        return {
            userinfo,
            video,
            tags,
            searchQuery,
            redirectToSearch,
            current_comment,
            send_comment,
            video_user_info,
            similar_video,
            comment_block,
            current_comment_block_index,
            prevBlock,
            nextBlock,
            user_video_relation,
            update_user_video_relation,
        };
    },
}).mount('#myapp');

// 头像点击跳转
document.querySelector('.avatar')?.addEventListener('click', function () {
    window.location.href = 'personal.html';
});