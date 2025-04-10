#include "playhand.h"
#include <QMap>
#include <QDebug>
#include <QtCore/QtCore>

PlayHand::PlayHand()
{
    m_laiZiCount=0;                       //癞子总共的张数
    m_tianLaiZiCount=0;                   //天癞子的张数
    m_diLaiZiCount=0;                   //地癞子的张数
}

Card::CardPoint Max(Card::CardPoint n1,Card::CardPoint n2){
    if(n1>=n2)return n1;
    else return n2;
}

PlayHand::PlayHand(HandType type, Card::CardPoint base, int extra)//构造函数
{
    m_handType = type;
    m_basePoint = base;
    m_extra = extra;
}

PlayHand::PlayHand(Cards& cards)
{

    m_laiZiCount=0;                       //癞子总共的张数
    m_tianLaiZiCount=0;                   //天癞子的张数
    m_diLaiZiCount=0;                   //地癞子的张数
    m_allCardCount=0;

    //获得整幅牌有多少张
    m_allCardCount=cards.cardCount();

    //初始化m_cardCount
    CardList cardList = cards.toCardList();//转换为有序的
    //初始化cardCount
    memset(m_cardCount, 0, sizeof(int) * Card::Card_End);
    //进入循环，计算每个点数有多少张,癞子一共有多少张
    CardList::const_iterator it = cardList.constBegin();
    for (; it != cardList.constEnd(); it++)
    {
        m_cardCount[(int)it->m_point]++;
        if(it->m_point==GameControl::TianLaiZi)m_tianLaiZiCount++;
        if(it->m_point==GameControl::DiLaiZi)m_diLaiZiCount++;
    }
    m_laiZiCount=m_tianLaiZiCount+m_diLaiZiCount;
    judgeHand(cards);
}

bool PlayHand::isPair(Cards& cards){
    //判断是不是对子
    if(m_allCardCount==2){
        for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]+m_laiZiCount==2){
                m_handType = Hand_Pair;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }
        }
    }
    return false;
}

bool PlayHand::isBombJokers(Cards& cards){
    //判断是不是王炸
    if(m_allCardCount==2){
        if(m_cardCount[Card::Card_SJ]==1&&m_cardCount[Card::Card_BJ]==1){
           m_handType = Hand_Bomb_Jokers;
           return true;
        }
    }
    return false;
}

bool PlayHand::isTriple(Cards& cards){
    if(m_allCardCount==3){
        for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]==3){//无癞子
                m_handType = Hand_Triple;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }//有癞子
            else if(m_cardCount[i]+m_laiZiCount==3&&m_laiZiCount!=0){
                m_handType = Hand_Triple;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }
        }
    }
    return false;

}

bool PlayHand::isBombJokersSingle(Cards& cards){
    if(m_allCardCount==3){
        if(m_cardCount[Card::Card_SJ]==1&&m_cardCount[Card::Card_BJ]==1){
            m_handType = Hand_Bomb_Jokers_Single;
            return true;
        }
    }
    return false;
}

bool PlayHand::isBoom(Cards& cards)
{
    if(m_allCardCount==4){
        if(m_laiZiCount==4){//癞子炸
            m_handType = Hand_Bomb_Laizi;
            m_basePoint=cards.maxPoint();
        }

        for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]==4&&m_laiZiCount==0){//不含癞子的炸
                m_handType = Hand_Bomb;
                m_basePoint=cards.maxPoint();
            }
            else if((m_cardCount[i]+m_laiZiCount)==4&&m_laiZiCount!=0){//含癞子的炸弹
                m_handType = Hand_Bomb_Virtual;
                m_basePoint=(Card::CardPoint)i;
            }
        }
    }
    return false;
}

bool PlayHand::isTripleSingle(Cards& cards){
    if(m_allCardCount==4){
        for(int i=Card::Card_End-1;i>=Card::Card_Begin;i--){
            if(m_cardCount[i]==3){//无癞子
                m_handType = Hand_Triple_Single;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }//一个癞子+一个单+一个对
            if(m_cardCount[i]==2&&m_laiZiCount==1){
                m_handType = Hand_Triple_Single;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }//两个癞子+两个单
            if(m_cardCount[i]==1&&m_laiZiCount==2&&i!=GameControl::TianLaiZi&&i!=GameControl::DiLaiZi){
                m_handType = Hand_Triple_Single;
                m_basePoint=(Card::CardPoint)i;//因为是从后往前遍历的，找到的i一定最大的
                return true;
            }
        }
    }
    return false;
}

bool PlayHand::isJockerBombPair(Cards& cards){
    if(m_allCardCount==4){
        if(m_cardCount[Card::Card_SJ]==1&&m_cardCount[Card::Card_BJ]==1){
           for(int i=Card::Card_End-1;i>=Card::Card_Begin;i--){
               if((m_cardCount[i]==1&&m_laiZiCount==1&&i!=GameControl::TianLaiZi&&i!=GameControl::DiLaiZi)
                       ||m_cardCount[i]==2){
                   m_handType = Hand_Bomb_Jokers_Pair;
                   return true;
           }
        }
    }

    }
    return false;
}

bool PlayHand::isJockerBombTwoSingle(Cards &cards){
    if(m_allCardCount==4){
        if(m_cardCount[Card::Card_SJ]==1&&m_cardCount[Card::Card_BJ]==1){
           for(int i=Card::Card_End-1;i>=Card::Card_Begin;i--){
               if(m_cardCount[i]==1&&m_laiZiCount==0){
                   m_handType = Hand_Bomb_Jokers_Two_Single;
                   return true;
               }
           }
        }
    }
    return false;
}

bool PlayHand::isShunZi(Cards &cards){
    QVector<int> shunziCount;
    int count=1;//顺子个数，初始化为1，因为单个牌就是1
    int countAll=0;
    int min=Card::Card_End;
    int max=Card::Card_Begin;
    for(int i=Card::Card_Begin;i<Card::Card_End-1;i++){
        if(m_cardCount[i]==1&&m_cardCount[i+1]==1){
            count++;
            if(i<min)min=i;
            if(i>max)max=i;
        }
        //遇到1，0的中断情况，pushback
        else if(m_cardCount[i]==1&&m_cardCount[i+1]==0)
        {
            shunziCount.push_back(count);
            count=1;
        }
    }
    for(int i=0;i<shunziCount.size();i++){
        countAll+=shunziCount[i];//计算总的连续数量
    }
    if(countAll+m_laiZiCount==(max-min+1)&&(max-min+1)>=5){//癞子能够补充它中断的个数,无癞子时allcount就是顺子数
        m_handType=Hand_Seq_Single;
        m_basePoint=(Card::CardPoint)min;
        m_extra=(max-min+1);
        return true;
    }
    return false;
}

bool PlayHand::isBombSingle(Cards& cards)
{
    if(m_allCardCount==5){
        if(m_laiZiCount==4){//癞子炸
            m_handType = Hand_Bomb_Single;
            m_basePoint=(Card::CardPoint)(GameControl::TianLaiZi>GameControl::DiLaiZi?GameControl::TianLaiZi:GameControl::DiLaiZi);
            return true;
        }
        for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]+m_laiZiCount==4){//不含癞子的炸
                m_handType = Hand_Bomb_Single;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }
            }
        }
    return false;
}


bool PlayHand::isPlane(Cards &cards){
    if(m_allCardCount==6){
        for(int i=Card::Card_Begin;i<Card::Card_End-1;i++){
            if(m_cardCount[i]==3&&m_cardCount[i+1]==3){
                m_handType=Hand_Plane;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }
            else if(m_cardCount[i]+m_laiZiCount==3&&m_laiZiCount!=0){//癞子只替代一种牌
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
                    if(m_cardCount[j]==3){
                        m_handType=Hand_Plane;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
                }
            }else if(m_cardCount[i]!=0&&m_cardCount[i+1]!=0
                     &&(m_cardCount[i]+m_cardCount[i+1]+m_laiZiCount==6))//仅由两种连续的+癞子组成
            {
                m_handType=Hand_Plane;
                m_basePoint=(Card::CardPoint)i;
                return true;
            }
        }
    }
    return false;
}

bool PlayHand::isLianDui(Cards &cards){
    QVector<int> laiDuiCount;
    int count=1;//连对个数，初始化为1，因为一对牌就是1
    int countAll=0;
    int min=Card::Card_End;
    int max=Card::Card_Begin;
    for(int i=Card::Card_Begin;i<Card::Card_End-1;i++){
        if(m_cardCount[i]==2&&m_cardCount[i+1]==2){
            count++;
            if(i<min)min=i;
            if(i>max)max=i;
        }
        //遇到1，0的中断情况，pushback，count重制为1
        else if(m_cardCount[i]==1&&m_cardCount[i+1]==0){
            laiDuiCount.push_back(count);
            count=1;
        }
    }
    for(int i=0;i<laiDuiCount.size();i++){
        countAll+=laiDuiCount[i];//计算总的连续数量
    }
    if(countAll*2+m_laiZiCount==(max-min+1)*2&&(max-min+1)>=5){//癞子能够补充它中断的个数,无癞子时allcount就是连对数
        m_handType=Hand_Seq_Pair;
        m_basePoint=(Card::CardPoint)min;
        m_extra=(max-min+1);
        return true;
    }
    return false;
}

bool PlayHand::isBoomPair(Cards& cards){
    if(m_allCardCount==6){
         for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]==4){
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--)
                    if(m_cardCount[j]+m_laiZiCount==2){//癞子替代pair
                        m_handType=Hand_Bomb_Pair;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
            }else if(m_cardCount[i]+m_laiZiCount==4&&m_laiZiCount!=0){//癞子替代炸弹
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--)
                    if(m_cardCount[j]==2)
                    {
                        m_handType=Hand_Bomb_Pair;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
            }else if(m_cardCount[i]!=0){
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--)//癞子替代了两者
                    if(m_cardCount[j]+m_cardCount[i]+m_laiZiCount==6){
                        m_handType=Hand_Bomb_Pair;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
            }

            }
         }
    return false;
}

bool PlayHand::isBoomTwoSingle(Cards &cards){
    if(m_allCardCount==6){
         for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]==4){//实炸和纯癞子炸都在
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--)
                    if(m_cardCount[j]==1&&m_laiZiCount==0){//两单，癞子不能存在
                        m_handType=Hand_Bomb_Two_Single;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
            }else if(m_cardCount[i]+m_laiZiCount==4&&m_laiZiCount!=0){//癞子只能替代炸弹
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--)
                    if(m_cardCount[j]==1&&j!=GameControl::TianLaiZi&&j!=GameControl::DiLaiZi)
                    {
                        for(int k=j-1;k>=Card::Card_Begin;k--){
                            //继续往下搜索，搜到不是癞子的为1为止
                            if(m_cardCount[k]==1&&k!=GameControl::TianLaiZi&&k!=GameControl::DiLaiZi){
                                m_handType=Hand_Bomb_Two_Single;
                                m_basePoint=(Card::CardPoint)i;
                                return true;
                            }
                        }
                    }
            }
            }
         }
    return false;
}

bool PlayHand::isBoomThriple(Cards& cards)
{
    if(m_allCardCount==7){
        for(int i=Card::Card_Begin;i<Card::Card_End-1;i++){
            if(m_cardCount[i]==4){
                //癞子替代3
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
                    if(m_cardCount[j]+m_laiZiCount==3){
                        m_handType=Hand_Bomb_thriple;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
                }
            }else if(m_cardCount[i]+m_laiZiCount==4&&m_laiZiCount!=0){//癞子代替炸弹
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
                    if(m_cardCount[j]==3){
                        m_handType=Hand_Bomb_thriple;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
                }
            }
            else if(m_cardCount[i]!=0){//癞子替代两者
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
                    if(m_cardCount[j]+m_cardCount[i]+m_laiZiCount==7&&m_laiZiCount!=0){
                        m_handType=Hand_Bomb_thriple;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
bool PlayHand::isPlaneTwoSingle(Cards& cards){//癞子只能替代飞机
    if(m_allCardCount==8){
        for(int i=Card::Card_Begin;i<Card::Card_End-1;i++){
            if(m_cardCount[i]==3&&m_cardCount[i+1]==3){
                for(int j=Card::Card_End-1;j>=Card::Card_Begin+1;j--){
                    if(m_cardCount[j]==1&&m_laiZiCount==0)
                        m_handType=Hand_Plane_Two_Single;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                }
                }else if(m_cardCount[i]+m_cardCount[i+1]+m_laiZiCount==6&&m_laiZiCount!=0){//癞子代替飞机
                for(int j=Card::Card_End-1;j>=Card::Card_Begin+1;j--){
                    if(m_cardCount[j]==1&&m_laiZiCount==0)
                        m_handType=Hand_Plane_Two_Single;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                }
            }
        }
    }
    return false;
}



bool PlayHand::isPlaneTwoPair(Cards& cards)
{
    int classCount=0;//非癞子以外有多少种牌
    if(m_allCardCount==10){
        for(int i=Card::Card_Begin;i<Card::Card_End-1;i++){
            if(m_cardCount[i]!=0&&i!=GameControl::TianLaiZi&&GameControl::DiLaiZi)classCount++;
            if(m_cardCount[i]==3&&m_cardCount[i+1]==3){
                for(int j=Card::Card_End-1;j>=Card::Card_Begin+1;j--){
                    if(m_cardCount[j]==2&&m_cardCount[j-1]==2){
                        //无任何癞子替代的
                        m_handType=Hand_Plane_Two_Pair;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
                }
            }
        }
        for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
            if(m_cardCount[j]!=0&&m_laiZiCount!=0&&classCount==4){
                if(m_cardCount[j]==2&&m_cardCount[j-1]==2){
                    //4种牌+癞子=10张，癞子填充全部
                    m_handType=Hand_Plane_Two_Pair;
                    m_basePoint=(Card::CardPoint)j;
                    return true;
            }
        }
    }
    }
    return false;
}

bool PlayHand::isTwoBoom(Cards& cards){
    int classCount=0;
    if(m_allCardCount==8){
        for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]!=0&&i!=GameControl::TianLaiZi&&i!=GameControl::DiLaiZi)classCount++;
            if(m_cardCount[i]==4){
                for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
                    if(m_cardCount[j]==4){
                        m_handType=Hand_Bomb_Two_Bomb;
                        m_basePoint=(Card::CardPoint)i;
                        return true;
                    }
                }
            }
        }
        if(classCount==2){
            for(int i=Card::Card_Begin;i<Card::Card_End;i++){
                if(m_cardCount[i]!=0&&i!=GameControl::TianLaiZi&&GameControl::DiLaiZi){
                    for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
                        if(m_cardCount[j]!=0&&j!=GameControl::TianLaiZi&&j!=GameControl::DiLaiZi){
                            if(m_cardCount[i]+m_cardCount[j]+m_laiZiCount==8){
                                m_handType=Hand_Bomb_Two_Bomb;
                                m_basePoint=(Card::CardPoint)i;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
}

bool PlayHand::isTriplePair(Cards& cards){
    if(m_allCardCount==5){//出现不了替代两者的情况，会被boom+1取代
        for(int j=Card::Card_End-1;j>=Card::Card_Begin;j--){
            if(m_cardCount[j]==3){
                for(int i=Card::Card_Begin;i<Card::Card_End;i++){
                    if(m_cardCount[i]+m_laiZiCount==2){//癞子全部替代pair
                        m_handType=Hand_Triple_Pair;
                        m_basePoint=(Card::CardPoint)j;
                        return true;
                    }
                }
            }else if(m_cardCount[j]+m_laiZiCount==3&&m_laiZiCount!=0){//全部替代3
                for(int i=Card::Card_Begin;i<Card::Card_End;i++){
                    if(m_cardCount[i]==2){
                        m_handType=Hand_Triple_Pair;
                        m_basePoint=(Card::CardPoint)j;
                        return true;
                    }
                }
            }
        }
    }
}

void PlayHand::judgeHand(Cards cards)
{
    m_handType = Hand_Unknown;
    m_basePoint = Card::Card_Begin;
    m_extra = 0;

    if(m_allCardCount==1){
        m_handType = Hand_Single;
        for(int i=Card::Card_Begin;i<Card::Card_End;i++){
            if(m_cardCount[i]==1)m_basePoint=(Card::CardPoint)i;
            return;
        }
    }
    if(isBombJokers(cards))return;
    if( isPair(cards))return;
    if(isBombJokersSingle(cards))return;
    if(isTriple(cards))return;
    if(isBoom(cards))return;
    if(isTripleSingle(cards))return;
    if(isJockerBombPair(cards))return;
    if(isJockerBombTwoSingle(cards))return;
    if(isTriplePair(cards))return;
    if(isShunZi(cards))return;
    if(isBombSingle(cards))return;
    if(isPlane(cards))return;
    if(isLianDui(cards))return;
    if(isBoomPair(cards))return;
    if(isBoomTwoSingle(cards))return;
    if(isBoomThriple(cards))return;
    if(isPlaneTwoSingle(cards))return;
    if(isPlaneTwoPair(cards))return;
    if(isTwoBoom(cards))return;

}

bool PlayHand::operator ==(const PlayHand& hand)
{
    return (m_handType == hand.m_handType &&
        m_basePoint == hand.m_basePoint &&
        m_extra == hand.m_extra);
}

PlayHand::HandType PlayHand::getType()
{
    return m_handType;
}

Card::CardPoint PlayHand::getBasePoint()
{
    return m_basePoint;
}

int PlayHand::getExtra()
{
    return m_extra;
}

bool PlayHand::operator>(const PlayHand& other) const
{
    if (m_handType == Hand_Unknown) return false;

    // 王炸无敌
    if (m_handType == Hand_Bomb_Jokers) return true;

    if (other.m_handType == Hand_Pass) return true;

    // 炸弹可炸普通牌
    if (other.m_handType >= Hand_Single &&
        other.m_handType <= Hand_Seq_Single &&
        m_handType == Hand_Bomb)
    {
        return true;
    }

    if (m_handType == other.m_handType)
    {
        if (m_handType == Hand_Seq_Pair || m_handType == Hand_Seq_Single)
        {
            return (m_basePoint > other.m_basePoint && m_extra == other.m_extra);
        }
        else
        {
            return (m_basePoint > other.m_basePoint);
        }
    }

    return false;
}

