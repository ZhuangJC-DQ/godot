def can_build(env, platform):
    # 控制模块在哪些平台上编译
    return True


def configure(env):
    # 配置编译选项、链接库等
    pass


def get_doc_classes():
    # 返回需要生成文档的类
    return [
        "GameFramework",
    ]


def get_doc_path():
    return "doc_classes"
