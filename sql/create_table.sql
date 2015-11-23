

#游戏区
CREATE TABLE IF NOT EXISTS zone
(
    id         BIGINT  NOT NULL PRIMARY KEY,       #区id
    opentime   INTEGER NOT NULL                   #unix_time
);


#角色
CREATE TABLE IF NOT EXISTS roleRarelyUp #很少或不更新的角色属性
(
    id						BIGINT      NOT NULL PRIMARY KEY,   #角色id
    name					VARCHAR(32) UNIQUE NOT NULL,		#角色名
    turnLife				TINYINT     NOT NULL DEFAULT 0,		#转生(1转、2转...)
    account					VARCHAR(32) NOT NULL,
    sex						TINYINT     DEFAULT 1, 
    job						TINYINT     DEFAULT 1,
	unlockCellNumOfRole		SMALLINT	NOT NULL DEFAULT 42,
    unlockCellNumOfHero		SMALLINT	NOT NULL DEFAULT 42,
	unlockCellNumOfStorage	SMALLINT	NOT NULL DEFAULT 70,
    defaultCallHero         TINYINT     NOT NULL DEFAULT 0,		#默认召唤的英雄
	guanzhiLevel			TINYINT     NOT NULL DEFAULT 0,		#官职等级
	buffer					Blob	    NOT NULL			#角色的缓存数据(供客户端使用)

	#zone					TINYINT     NOT NULL
    #platform				TINYINT		NOT NULL
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS roleOftenUp  #经常更新的角色属性
(
    id						BIGINT      NOT NULL PRIMARY KEY,   #角色id
    level					INT			NOT NULL DEFAULT 1,		#当前等级
	exp						BIGINT		NOT NULL DEFAULT 0,		#累计经验值
	money_1					BIGINT      NOT NULL DEFAULT 0,		#绑定金币
	money_2					BIGINT      NOT NULL DEFAULT 0,		#非绑金币
	money_3					BIGINT		NOT NULL DEFAULT 0,		#绑定元宝
	money_4					BIGINT      NOT NULL DEFAULT 0,		#非绑元宝
	money_5					BIGINT		NOT NULL DEFAULT 0,		#声望 
	money_6					BIGINT		NOT NULL DEFAULT 0,		#强化值
	money_7                 BIGINT      NOT NULL DEFAULT 0,		#战功
	money_8					BIGINT		NOT NULL DEFAULT 0,		#角色灵力值
	money_9					BIGINT		NOT NULL DEFAULT 0,		#英雄灵力值
	money_10				BIGINT		NOT NULL DEFAULT 0,		#龙魂
	FOREIGN KEY (id) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS roleOfflnUp  #下线更新的角色属性
(
    id						BIGINT      NOT NULL PRIMARY KEY,   #角色id
	mp                      INT         NOT NULL DEFAULT 0,
    hp                      INT         NOT NULL DEFAULT 0,
    dir                     TINYINT     NOT NULL DEFAULT 0,
    sceneId                 BIGINT      NOT NULL DEFAULT 0,
    pos                     INT         NOT NULL DEFAULT 0,
    preSceneId              BIGINT      NOT NULL DEFAULT 0,
    prePos                  INT         NOT NULL DEFAULT 0,
    dead                    bool        NOT NULL DEFAULT false,
    deathTime               INT         NOT NULL DEFAULT 0,
    totalOnlineSec          INT         NOT NULL DEFAULT 0,
    offlnTime               INT         NOT NULL DEFAULT 0,
    evilVal                 SMALLINT    NOT NULL DEFAULT 0,
    attackMode              TINYINT     NOT NULL DEFAULT 1,
	FOREIGN KEY (id) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#帮会表
CREATE TABLE IF NOT EXISTS faction
(
    factionId				BIGINT      NOT NULL PRIMARY KEY,   #角色id
    name                    VARCHAR(32) UNIQUE NOT NULL,
    level                   INT         NOT NULL DEFAULT 1,
    exp                     BIGINT      NOT NULL DEFAULT 0,
    resource                BIGINT      NOT NULL DEFAULT 0,
    leader                  BIGINT      NOT NULL DEFAULT 0,
    viceLeaders             Blob        NOT NULL,
    warriorLeader           BIGINT      NOT NULL DEFAULT 0,
    magicianLeader          BIGINT      NOT NULL DEFAULT 0,
    taoistLeader            BIGINT      NOT NULL DEFAULT 0,
    notice                  Text        NOT NULL
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS roleInFaction
(
    id						BIGINT      NOT NULL PRIMARY KEY,   #角色id
    factionId               BIGINT      NOT NULL DEFAULT 0,
    banggong                BIGINT      NOT NULL DEFAULT 0,
	FOREIGN KEY (id) REFERENCES roleRarelyUp (id),
	FOREIGN KEY (factionId) REFERENCES faction (factionId)
) CHARSET=utf8;

#物品表
CREATE TABLE IF NOT EXISTS object(
	objId		BIGINT		NOT NULL,			#相对role唯一的objId
	roleId		BIGINT		NOT NULL,
	packageType TINYINT		NOT NULL,
	tplId		INT			NOT NULL,
	item		SMALLINT	NOT NULL,
	cell		SMALLINT	NOT NULL,
	skillId		INT			NOT NULL DEFAULT 0, #极品装备触发的被动技能Id(几率触发)    
    bind        TINYINT     NOT NULL,
    sellTime    INT         NOT NULL,
	strongLevel TINYINT     NOT NULL DEFAULT 0,
	luckyLevel	TINYINT		NOT NULL DEFAULT 0,
	PRIMARY KEY (objId, roleId),
	FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#技能表
CREATE TABLE IF NOT EXISTS skill(
    roleId          BIGINT      NOT NULL,
    skillId         INT         NOT NULL,
    skillLv         INT         NOT NULL DEFAULT 1,
    strengthenLv    INT         NOT NULL DEFAULT 0,
    exp             INT         NOT NULL DEFAULT 0,
    PRIMARY KEY (roleId, skillId),
    FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS friends(
    roleId      BIGINT      NOT NULL,
    friendStr   Blob      NOT NULL,
    PRIMARY KEY (roleId)
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS blacklist(
    roleId      BIGINT      NOT NULL,
    blackId     BIGINT      NOT NULL,
    PRIMARY KEY (roleId,blackId)
) CHARSET=utf8;

#buff表
CREATE TABLE IF NOT EXISTS buff(
    roleId          BIGINT      NOT NULL,
    buffId          INT         NOT NULL,
    sec             INT         NOT NULL DEFAULT 0,
    endtime         INT         NOT NULL DEFAULT 0,
    dur             INT         NOT NULL DEFAULT 0,
    PRIMARY KEY (roleId, buffId),
    FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS enemy(
    roleId      BIGINT      NOT NULL,
    enemyId     Blob        NOT NULL,
    PRIMARY KEY (roleId)
) CHARSET=utf8;

#英雄表
CREATE TABLE IF NOT EXISTS hero(
	job			TINYINT	    NOT NULL,		
	roleId		BIGINT		NOT NULL,
    sex			TINYINT	    NOT NULL,
	level       INT			NOT NULL,
    exp         BIGINT      NOT NULL DEFAULT 0,
    hp          INT         NOT NULL DEFAULT 0,
    mp          INT         NOT NULL DEFAULT 0,
    turnLife	TINYINT     NOT NULL DEFAULT 0,		#转生(1转、2转...)
    clother     INT         NOT NULL DEFAULT 0,     #衣服id
	PRIMARY KEY (job, roleId),
	FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#用户计数器
CREATE TABLE IF NOT EXISTS roleCounter(
    roleId          BIGINT      NOT NULL,
    counterStr      Blob        NOT NULL,
    PRIMARY KEY (roleId),
    FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

############heroData

#hero技能表
CREATE TABLE IF NOT EXISTS heroSkill(
    roleId          BIGINT      NOT NULL,
    job             TINYINT     NOT NULL,
    skillId         INT         NOT NULL,
    skillLv         INT         NOT NULL DEFAULT 1,
    strengthenLv    INT         NOT NULL DEFAULT 0,
    exp             INT         NOT NULL DEFAULT 0,
    PRIMARY KEY (roleId, job, skillId),
    FOREIGN KEY (job,roleId) REFERENCES hero (job, roleId)
) CHARSET=utf8;

#herobuff表
CREATE TABLE IF NOT EXISTS heroBuff(
    roleId          BIGINT      NOT NULL,
    job             TINYINT     NOT NULL,
    buffId          INT         NOT NULL,
    sec             INT         NOT NULL DEFAULT 0,
    endtime         INT         NOT NULL DEFAULT 0,
    dur             INT         NOT NULL DEFAULT 0,
    PRIMARY KEY (roleId, job, buffId),
    FOREIGN KEY (job, roleId) REFERENCES hero (job, roleId)
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS mail(
    roleId          BIGINT      NOT NULL,
    mailIndex       INT         NOT NULL DEFAULT 1,
    title           TEXT        NOT NULL,
    text            TEXT        NOT NULL,
    state           TINYINT     NOT NULL DEFAULT 1,
    time            INT         NOT NULL,
    obj             TEXT        NOT NULL,
    PRIMARY KEY (roleId, mailIndex),
    FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

CREATE TABLE IF NOT EXISTS horse(
    roleId          BIGINT      NOT NULL,
    blobStr         blob        NOT NULL,
    PRIMARY KEY (roleId),
    FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#称号表
CREATE TABLE IF NOT EXISTS title(
    roleId          BIGINT      NOT NULL,
	titleId			INT         NOT NULL,
	titleType		TINYINT		NOT NULL,
    createTime		INT			NOT NULL,
    disableTime		INT			NOT NULL,
	used            BOOL        NOT NULL DEFAULT FALSE,
	PRIMARY KEY (roleId, titleId),
    FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#洗练表
CREATE TABLE IF NOT EXISTS wash(
	roleId			BIGINT		NOT NULL,
	sceneItem		TINYINT		NOT NULL,
	washType		TINYINT		NOT NULL,
	propStr			TEXT		NOT NULL,
	PRIMARY KEY (roleId, sceneItem, washType),
	FOREIGN KEY	(roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#任务表
CREATE TABLE IF NOT EXISTS task(
	roleId			BIGINT		NOT NULL,
    blobStr         blob        NOT NULL,
	PRIMARY KEY (roleId),
	FOREIGN KEY	(roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#帮派任务
CREATE TABLE IF NOT EXISTS factionTask(
    roleId          BIGINT      NOT NULL,
    data            blob                ,
    PRIMARY KEY (roleId),
    FOREIGN KEY (roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#龙珠
CREATE TABLE IF NOT EXISTS dragonBall(
	roleId		    BIGINT		NOT NULL,
	dragonType		TINYINT		NOT NULL,
	exp				INT			NOT NULL DEFAULT 0,
	PRIMARY KEY (roleId, dragonType),
	FOREIGN KEY	(roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#经验区(泡点)
CREATE TABLE IF NOT EXISTS expArea(
	roleId			BIGINT		NOT NULL,
	expType			TINYINT		NOT NULL,
	sec				INT			NOT NULL DEFAULT 0,
	PRIMARY KEY (roleId, expType),
	FOREIGN KEY	(roleId) REFERENCES roleRarelyUp (id)
) CHARSET=utf8;

#全局杂项数据存储
CREATE TABLE IF NOT EXISTS globalSundry(
    id              SMALLINT    NOT NULL DEFAULT 1,
    blobStr         Blob        NOT NULL,
	PRIMARY KEY (id)
) CHARSET=utf8;

#频繁角色杂项数据存储
CREATE TABLE IF NOT EXISTS timerSundry(
    roleId      BIGINT          NOT NULL,
    data        Blob            NOT NULL,
    PRIMARY KEY (roleId)
) CHARSET=utf8;

#角色杂项数据存储
CREATE TABLE IF NOT EXISTS sundry(
    roleId      BIGINT          NOT NULL,
    data        Blob            NOT NULL,
    PRIMARY KEY (roleId)
) CHARSET=utf8;

#天下第一
CREATE TABLE IF NOT EXISTS first(
	roleId			BIGINT		NOT NULL,
	name			VARCHAR(32)	NOT NULL,
	job				TINYINT		NOT NULL,
	sex				TINYINT     NOT NULL,
	PRIMARY KEY (roleId)
) CHARSET=utf8;

#摆摊出售记录表
CREATE TABLE IF NOT EXISTS stallSellLog(
    roleId          BIGINT      NOT NULL,
    blobStr         Blob        NOT NULL,
    PRIMARY KEY (roleId)
) CHARSET=utf8


