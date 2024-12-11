const { ref, onMounted } = Vue;

const home = Vue.createApp({
    setup() {
        const videoList = ref([]);//视频列表
        const page = ref([]); //分页
        const currentPage = ref(0); //当前页码
        const searchQuery = ref('');//搜索框中的内容
        const sortType = ref('comprehensive');//排序类型
        const userinfo = ref({});//用户信息
        const video_weight = ref({
            views: 1,//播放量
            release_time: 2,//发布时间
            comments: 3,//评论
            favorites: 4,//收藏
        });//权重

        //获取用户信息
        const getUserinfo = () => {
            axios.get('/user/getUserInfo').then(response => {
                userinfo.value = response.data;
            }).catch(error => {
                console.error('Error fetching user info:', error);
            });
        }

        //获取搜索框中的内容
        const getSearchQuery = () => {
            const params = new URLSearchParams(window.location.search);
            searchQuery.value = params.get('search');
        }

        //搜索跳转
        const redirectToSearch = () => {
            window.location.href = `search.html?search=${searchQuery.value}`;
        }

        //获取视频列表
        const getVideoList = () => {
            axios.get(`/video/select?search=${searchQuery.value}`).then(response => {
                videoList.value = response.data;
                filterByType();
                sortBy(sortType.value);
                getPage();
            }).catch(error => {
                console.error('Error fetching video list:', error);
            });
        }

        //实现分页功能，一次显示20个视频
        const getPage = () => {
            page.value = [];
            let num = filteredVideoList.value.length;
            while (num > 0) {
                page.value.push(filteredVideoList.value.slice(0, 20));
                num -= 20;
            }
        }

        //上一页
        const prevPage = () => {
            if (currentPage.value > 0) {
                currentPage.value--;
            } else {
                alert("已经是第一页了");
            }
        }

        //下一页
        const nextPage = () => {
            if (currentPage.value < page.value.length - 1) {
                currentPage.value++;
            } else {
                alert("已经是最后一页了");
            }
        }

        //计算一个视频的权重
        const calculateWeight = (video) => {
            let weight = 0;
            //枚举权重对象
            weight += video_weight.value.views * video.views;//播放量
            weight += video_weight.value.release_time * new Date(video.release_time).getTime();//发布时间
            weight += video_weight.value.comments * video.comments;//评论
            weight += video_weight.value.favorites * video.favorites;//收藏
            return weight
        }

        //排序，默认是综合排序
        const sortBy = (type) => {
            if (filteredVideoList.value.length > 0) {
                filteredVideoList.value.sort((a, b) => {
                    switch (type) {
                        case 'views':
                            return b.views - a.views; // 播放量从高到低
                        case 'release_time':
                            return new Date(b.release_time) - new Date(a.release_time); // 发布时间从近到远
                        case 'comments':
                            return b.comments - a.comments; // 弹幕量从高到低
                        case 'favorites':
                            return b.favorites - a.favorites; // 收藏量从高到低
                        case 'comprehensive':
                            return calculateWeight(b) - calculateWeight(a); // 综合排序，按照权重
                        default:
                            return 0;
                    }
                });
                //排序后重新获取分页
                getPage();
            }
        }

        const totalType = ref([]);
        const getTotalType = () => {
            axios.get(`/getTotal?table_name=type`).then(response => {
                totalType.value = response.data;
                console.log(totalType.value);
            }).catch(error => {
                console.error('Error fetching total type:', error);
            });
        }

        // 新增的变量
        const selectedType = ref('All');
        //分类后的视频列表
        const filteredVideoList = ref([]);
        // 新增的类型过滤方法
        const filterByType = () => {
            if (selectedType.value === 'All') {
                filteredVideoList.value = videoList.value;
                getPage();
                console.log(filteredVideoList.value);
            } else {
                console.log(selectedType.value);
                filteredVideoList.value = videoList.value.filter(video => video.video_type_id === selectedType.value);
                getPage();
            }
        }


        onMounted(() => {
            getSearchQuery();
            getUserinfo();
            getVideoList();
            getTotalType();
        });

        return {
            searchQuery,
            redirectToSearch,
            page,
            currentPage,
            prevPage,
            nextPage,
            sortBy,
            sortType,
            userinfo,
            totalType,
            selectedType,
            filterByType,
            filteredVideoList
        }
    }
}).mount('#home');

// 头像点击跳转
document.querySelector('.avatar')?.addEventListener('click', function () {
    window.location.href = 'personal.html';
});
