
#include "Scene_Play.hpp"

Scene_Play::Scene_Play(SceneHandler &sceneHandler, Resources &resources, MapData *mapData, std::vector<std::list<UnitData>> *unitsData) : Scene(sceneHandler, resources), _mapData(*mapData), _unitsData(*unitsData) {}

Scene_Play::~Scene_Play()
{
    _resources.Music("ambient").stop();
}

void Scene_Play::init()
{
    t_title.setFont(_resources.Font("font1"));
    t_title.setString("Press 'e' to exit");
    t_title.setCharacterSize(30);
    t_title.setFillColor(sf::Color::White);
    t_title.setPosition(10, 700);

    t_currentTeam.setFont(_resources.Font("font1"));
    t_currentTeam.setCharacterSize(40);
    t_currentTeam.setStyle(sf::Text::Style::Bold);
    t_currentTeam.setFillColor(sf::Color::Yellow);
    t_currentTeam.setPosition(12, 606);

    rs_info.setPosition(105, 620);
    rs_info.setSize(sf::Vector2f(472, 175));
    rs_info.setFillColor(sf::Color(0, 51, 102));
    rs_info.setOutlineThickness(-1.f);
    rs_info.setOutlineColor(sf::Color::Yellow);

    t_dataUnit = std::vector<sf::Text>(DataUnit::DU_ELEMS);

    for(int i = 0; i < DataUnit::DU_ELEMS; ++i)
    {
        t_dataUnit[i].setFont(_resources.Font("font1"));
        t_dataUnit[i].setCharacterSize(32);
        t_dataUnit[i].setString("---");
    }

    t_dataUnit[DataUnit::DU_NAME].setPosition(120, 610);
    t_dataUnit[DataUnit::DU_TEAM].setPosition(270, 610);
    t_dataUnit[DataUnit::DU_HP].setPosition(120, 645);
    t_dataUnit[DataUnit::DU_RESIST_F].setPosition(120, 661);
    t_dataUnit[DataUnit::DU_RESIST_M].setPosition(120, 677);
    t_dataUnit[DataUnit::DU_WEAPON].setPosition(250, 645);
    t_dataUnit[DataUnit::DU_WEAPON_TYPE].setPosition(270, 661);
    t_dataUnit[DataUnit::DU_WEAPON_RANGE].setPosition(270, 677);
    t_dataUnit[DataUnit::DU_WEAPON_SPECIAL_RANGE].setPosition(270, 693);
    t_dataUnit[DataUnit::DU_MOVEMENT].setPosition(250, 721);
    t_dataUnit[DataUnit::DU_MOVEMENT_RANGE].setPosition(270, 737);
    t_dataUnit[DataUnit::DU_MOVEMENT_SPECIAL_RANGE].setPosition(270, 753);
    t_dataUnit[DataUnit::DU_GOD].setPosition(300, 610);
    t_dataUnit[DataUnit::DU_GOD].setString("");

    t_infoAbility.setFont(_resources.Font("font1"));
    t_infoAbility.setString("[A] to ABILITY");
    t_infoAbility.setCharacterSize(30);
    t_infoAbility.setFillColor(sf::Color::Yellow);
    t_infoAbility.setPosition(600, 640);

    t_infoPassives.setFont(_resources.Font("font1"));
    t_infoPassives.setString("[S/D] to surf PASSIVES");
    t_infoPassives.setCharacterSize(30);
    t_infoPassives.setFillColor(sf::Color::Green);
    t_infoPassives.setPosition(600, 660);

    t_infoBack.setFont(_resources.Font("font1"));
    t_infoBack.setString("[B] to go BACK");
    t_infoBack.setCharacterSize(30);
    t_infoBack.setFillColor(sf::Color::Blue);
    t_infoBack.setPosition(600, 680);

    t_infoFinish.setFont(_resources.Font("font1"));
    t_infoFinish.setString("[F] to FINISH");
    t_infoFinish.setCharacterSize(30);
    t_infoFinish.setFillColor(sf::Color::Red);
    t_infoFinish.setPosition(600, 700);

    

    _passives = List(sf::FloatRect(430, 627, 140, 160), 3);
    _abilities = List(sf::FloatRect(600, 625, 175, 165), 2, 1);

    _map = Map(sf::FloatRect(10, 10, 780, 600));

    int numTeams = _unitsData.size();

    _teams = std::vector<std::vector<Unit>>(numTeams);

    for (int i = 0; i < numTeams; ++i)
    {
        _teams[i] = std::vector<Unit>(_unitsData[i].size());

        int j = 0;

        std::list<UnitData>::iterator it = _unitsData[i].begin();
        while (it != _unitsData[i].end())
        {
            if (_map.correctCoord(_mapData._teams[i][j])) _teams[i][j].init(*it, i, _mapData._teams[i][j]);
            else std::cerr << "ERROR: Bad <" << it->_name << "> unit position coord" << std::endl;
            ++j;
            ++it;
        }
    }

    _map.setMap(_resources, _mapData, _teams);

    _resources.Music("ambient").setLoop(true);
    _resources.Music("ambient").play();

    _selected = false;

    initPhase(0);
}

void Scene_Play::handleEvents(const sf::Event &event)
{
    switch (event.type)
    {
        case sf::Event::KeyPressed:
        {
            switch (event.key.code)
            {
                case sf::Keyboard::E:
                    _sceneHandler.popScene();
                    break;
                case sf::Keyboard::P:
                    if(_selected) _currentUnit->_passives.insert(_currentUnit->_passives.end(), Passive("test", -1, {TarjetTeam::TT_ALLY}, {Modification(UnitAttribute::UA_HP, true, -20, true, {}, {std::make_pair(UnitAttribute::UA_RESIST_F, 1.f)})}));
                    break;
                case sf::Keyboard::S:
                    _passives.up();
                    break;
                case sf::Keyboard::D:
                    _passives.down();
                    break;
                case sf::Keyboard::Up:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_BEGIN:
                        case TurnPhase::TP_SELECTED:
                        case TurnPhase::TP_ACTION:
                        {
                            if (_map.pointer().y > 0)
                            {
                                --_map.pointer().y;
                                if(!_map.getCell(_map.pointer()).empty()) setDataUnit(*_map.getCell(_map.pointer())._unit);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                case sf::Keyboard::Left:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_BEGIN:
                        case TurnPhase::TP_SELECTED:
                        case TurnPhase::TP_ACTION:
                        {
                            if (_map.pointer().x > 0)
                            {
                                --_map.pointer().x;
                                if(!_map.getCell(_map.pointer()).empty()) setDataUnit(*_map.getCell(_map.pointer())._unit);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                case sf::Keyboard::Right:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_BEGIN:
                        case TurnPhase::TP_SELECTED:
                        case TurnPhase::TP_ACTION:
                        {
                            if (_map.pointer().x < int(_map._WCells) - 1)
                            {
                                ++_map.pointer().x;
                                if(!_map.getCell(_map.pointer()).empty()) setDataUnit(*_map.getCell(_map.pointer())._unit);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                case sf::Keyboard::Down:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_BEGIN:
                        case TurnPhase::TP_SELECTED:
                        case TurnPhase::TP_ACTION:
                        {
                            if (_map.pointer().y < int(_map._HCells) - 1)
                            {
                                ++_map.pointer().y;
                                if(!_map.getCell(_map.pointer()).empty()) setDataUnit(*_map.getCell(_map.pointer())._unit);
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                case sf::Keyboard::Space:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_BEGIN:
                        {
                            if(!_map.getCell(_map.pointer()).empty() && _map.getCell(_map.pointer())._unit->_team == _currentTeam && _map.getCell(_map.pointer())._unit->_active)
                            {
                                bool tarjets = !_map.getCell(_map.pointer())._unit->_states[UnitState::FIXED] && _map.selectCell(_map.pointer());
                                if(tarjets)
                                {
                                    _selected = true;
                                    _currentUnit = _map.getCell(_map.selector())._unit;
                                    _currentTurnPhase = TurnPhase::TP_SELECTED;
                                }
                                else
                                {
                                    _map.eraseSelection();
                                    _map.selectCell(_map.pointer(), false);
                                    _currentTurnPhase = TurnPhase::TP_ACTION;
                                    std::cerr << "No movement tarjets" << std::endl;
                                }
                            }
                        }
                            break;
                        case TurnPhase::TP_SELECTED:
                        {
                            if(_map.getCell(_map.pointer())._action == ActionType::AT_MOVE)
                            {
                                _map.eraseSelection();
                                _map.moveUnit(_currentUnit, _map.pointer());
                                _map.selectCell(_map.pointer(), false);
                                _currentTurnPhase = TurnPhase::TP_ACTION;
                            }
                        }
                            break;
                        case TurnPhase::TP_ACTION:
                        {
                            if(_map.getCell(_map.pointer())._action != ActionType::AT_NONE)
                            {
                                if(_map.getCell(_map.pointer())._action == ActionType::AT_ENEMY)
                                {
                                    std::cerr << "ACTION TO ENEMY" << std::endl;
                                }   
                                else
                                {
                                    std::cerr << "ACTION TO ALLY" << std::endl;
                                }   
                                
                                _currentUnit->_active = false;
                                _currentUnit->_base._sprite.setColor(sf::Color::White);
                                _map.eraseSelection();
                                _selected = false;
                                --_remainUnits;
                                if(_remainUnits == 0) initPhase((++_currentTeam)%_teams.size());
                                _currentTurnPhase = TurnPhase::TP_BEGIN;
                            }
                        }
                            break;
                        case TurnPhase::TP_ABILITY:
                        {
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                case sf::Keyboard::B:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_SELECTED:
                        {
                            _map.eraseSelection();
                            _selected = false;
                            _currentTurnPhase = TurnPhase::TP_BEGIN;
                        }
                            break;
                        case TurnPhase::TP_ABILITY:
                        {
                            _map.selectCell(_currentUnit->_position, false);
                            _currentTurnPhase = TurnPhase::TP_ACTION;
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                case sf::Keyboard::A:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_ACTION:
                        {
                            _map.eraseSelection();
                            _currentTurnPhase = TurnPhase::TP_ABILITY;
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                case sf::Keyboard::F:
                {
                    switch(_currentTurnPhase)
                    {
                        case TurnPhase::TP_BEGIN:
                        {
                            initPhase((++_currentTeam)%_teams.size());
                            _currentTurnPhase = TurnPhase::TP_BEGIN;
                        }
                            break;
                        case TurnPhase::TP_ACTION:
                        {
                            _currentUnit->_active = false;
                            _currentUnit->_base._sprite.setColor(sf::Color(102, 102, 102));
                            _map.eraseSelection();
                            _selected = false;
                            --_remainUnits;
                            if(_remainUnits == 0) initPhase((++_currentTeam)%_teams.size());
                            _currentTurnPhase = TurnPhase::TP_BEGIN;                            
                        }
                            break;
                        default:
                            break;
                    }
                }
                    break;
                default:
                    break;
            }
        }
            break;
        default:
            break;
    }
}

void Scene_Play::update(const sf::Time deltatime)
{
    _map.update(deltatime);
    _passives.update(deltatime);

    if(_currentTurnPhase == TurnPhase::TP_ABILITY) _abilities.update(deltatime);

    for(unsigned int i = 0; i < _teams[_currentTeam].size(); ++i)
    {
        if(_teams[_currentTeam][i]._active) _teams[_currentTeam][i]._base._sprite.updateAnimation(deltatime);
    }

    t_title.setString(std::to_string(_map.getCell(_map.pointer())._checked) + " " + std::to_string(_map.getCell(_map.pointer())._distance) + " " + std::to_string(_map.getCell(_map.pointer())._action));
}

void Scene_Play::draw(sf::RenderWindow& window) const
{
    window.draw(rs_info);
    window.draw(t_title);
    window.draw(t_currentTeam);


    if(_currentTurnPhase != TurnPhase::TP_SELECTED) window.draw(t_infoFinish);
    if(_currentTurnPhase == TurnPhase::TP_ACTION) window.draw(t_infoAbility);
    window.draw(t_infoPassives);
    if(_currentTurnPhase == TurnPhase::TP_SELECTED) window.draw(t_infoBack);

    if(_currentTurnPhase == TurnPhase::TP_ABILITY) _abilities.draw(window);

    for(const sf::Text& i : t_dataUnit)
    {
        window.draw(i);
    }
    _passives.draw(window);

    _map.draw(window);
}

void Scene_Play::pause() {}

void Scene_Play::resume() {}

void Scene_Play::initPhase(unsigned int team)
{
    _currentTeam = team;
    _remainUnits = 0;
    t_currentTeam.setString("Team " + std::to_string(team));

    for(unsigned int i = 0; i < _teams.size(); ++i)
    {
        for(unsigned int j = 0; j < _teams[i].size(); ++j)
        {
            _teams[i][j]._active = false;
            _teams[i][j]._base._sprite.setColor(sf::Color::White);
            _teams[i][j]._base._sprite.stopAnimation();

            if(i == team) _teams[i][j].update(TarjetTeam::TT_ALLY);
            else _teams[i][j].update(TarjetTeam::TT_ENEMY);
        }
    }

    for(unsigned int i = 0; i < _teams[team].size(); ++i)
    {
        if(_teams[team][i]._alive)
        {
            if(!_teams[team][i]._states[UnitState::DAZED])
            {
                _teams[team][i]._active = true;
                _teams[team][i]._base._sprite.setColor(sf::Color(153, 255, 153));
                ++_remainUnits;
                _teams[team][i]._base._sprite.setActiveAnimation("idle");
                _teams[team][i]._base._sprite.playAnimation();
            }
            else
            {
                _teams[team][i]._active = false;
                _teams[team][i]._base._sprite.setColor(sf::Color(255, 255, 153));
            }
        }
    }

    _currentUnit = nullptr;
    _currentTurnPhase = TurnPhase::TP_BEGIN;
}

void Scene_Play::setDataUnit(const Unit& unit)
{
    t_dataUnit[DataUnit::DU_NAME].setString("NAME: " + unit._base._name);
    t_dataUnit[DataUnit::DU_TEAM].setString("TEAM: " + std::to_string(unit._team));
    t_dataUnit[DataUnit::DU_HP].setString("HP: " + std::to_string(unit._attributes[UnitAttribute::UA_HP]));
    t_dataUnit[DataUnit::DU_RESIST_F].setString("RES(F): " + std::to_string(unit._attributes[UnitAttribute::UA_RESIST_F]));
    t_dataUnit[DataUnit::DU_RESIST_M].setString("RES(M): " + std::to_string(unit._attributes[UnitAttribute::UA_RESIST_M]));
    t_dataUnit[DataUnit::DU_WEAPON].setString("WEAPON: " + unit._base._weapon._name);
    t_dataUnit[DataUnit::DU_WEAPON_TYPE].setString("Type: " + WT_Strings[unit._base._weapon._type]);

    std::string tmp = "";
    std::set<int>::const_iterator it = unit._base._weapon._range.begin();
    while(it != unit._base._weapon._range.end())
    {
        tmp += " ";
        tmp += std::to_string(*it);

        ++it;
    }

    t_dataUnit[DataUnit::DU_WEAPON_RANGE].setString("Range:" + tmp);

    tmp = "";
    std::vector<Coord>::const_iterator it1 = unit._base._weapon._specialRange.begin();
    while(it1 != unit._base._weapon._specialRange.end())
    {
        tmp += " ";
        tmp += "(" + std::to_string(it1->x) + "," + std::to_string(it1->y) + ")";
        ++it1;
    }

    t_dataUnit[DataUnit::DU_WEAPON_SPECIAL_RANGE].setString("S.Range:" + tmp);

    t_dataUnit[DataUnit::DU_MOVEMENT].setString("MOVEMENT: " + MT_Strings[unit._movementType]);

    tmp = "";
    it = unit._movementRange.begin();
    while(it != unit._movementRange.end())
    {
        tmp += " ";
        tmp += std::to_string(*it);

        ++it;
    }

    t_dataUnit[DataUnit::DU_MOVEMENT_RANGE].setString("Range: " + tmp);

    tmp = "";
    it1 = unit._specialMovementRange.begin();
    while(it1 != unit._specialMovementRange.end())
    {
        tmp += " ";
        tmp += "(" + std::to_string(it1->x) + "," + std::to_string(it1->y) + ")";
        ++it1;
    }

    t_dataUnit[DataUnit::DU_MOVEMENT_SPECIAL_RANGE].setString("S.Range:" + tmp);
    
    tmp = "";
    if(unit._states[UnitState::GOD]) tmp = "#GOD#";
    t_dataUnit[DataUnit::DU_GOD].setString(tmp);


    _passives.clear();

    std::list<Passive>::const_iterator it2 = unit._passives.begin();
    while(it2 != unit._passives.end())
    {
        Item tmp;

        sf::Text text;
        text.setFont(_resources.Font("font1"));
        text.setCharacterSize(40);

        std::string str = " [";
        std::set<TarjetTeam>::iterator it3 = it2->_triggerTurns.begin();
        while(it3 != it2->_triggerTurns.end())
        {
            str += " ";
            str += TT_Strings[*it3];
            ++it3;
        }
        str += " ]";

        text.setString("#" + it2->_name + "#" + str);
        tmp.addText(sf::Vector2f(14, -10), text);

        for(unsigned int i = 0; i < it2->_modifications.size(); ++i)
        {
            if(it2->_modifications[i]._modAttributes)
            {
                str = std::to_string(it2->_modifications[i]._aValue) + " " + UA_Strings[it2->_modifications[i]._aTarjet] + " | ";

                for(unsigned int j = 0; j < it2->_modifications[i]._aPro.size(); ++j) str +=  "+" + UA_Strings[it2->_modifications[i]._aPro[j].first];
                for(unsigned int j = 0; j < it2->_modifications[i]._aCont.size(); ++j) str +=  "-" + UA_Strings[it2->_modifications[i]._aCont[j].first];

                text.setString(str);
                tmp.addText(sf::Vector2f(14, 14 + 14*i), text);
            }

            if(it2->_modifications[i]._modStates)
            {
                text.setString(std::to_string(it2->_modifications[i]._sValue) + " " + US_Strings[it2->_modifications[i]._sTarjet]);
                tmp.addText(sf::Vector2f(14, 14 + 14*i), text);
            }
        }

        _passives.add(tmp);

        ++it2;
    }
}
