#ifndef VLUA_APIDOC

%nodefaultctor vHavokBehaviorComponent;
%nodefaultdtor vHavokBehaviorComponent;

class vHavokBehaviorComponent : public IVObjectComponent
{
public:
   %rename(Remove) DisposeObject();
   void DisposeObject();
	
   bool IsNodeActive(const char* nodeName);
   void SetFloatVar(const char* variableName, float value);
   void SetWordVar(const char* variableName, int value);
   void SetBoolVar(const char* variableName, bool value);
   bool GetBoolVar(const char* variableName) const;
   void GetBoneTransform( const char* boneName, hkvVec3& outPos, hkvMat3& outRot );
   void TriggerEvent(const char* eventName) const;
   void RegisterEventHandler(const char* eventName);
   bool WasEventTriggered(const char* eventName) const;
    
   %extend{  
   void TempMeth( hkvVec3 center )
		{
		}
		
	VSWIG_CREATE_CAST(vHavokBehaviorComponent)
   }
};

#else

/// \brief Behavior component class (Havok): Object component wrapper class that provides a Behavior functionality for an entity.
class vHavokBehaviorComponent : public IVObjectComponent
{
public:
	
  /// @}
  /// @name Variables management
  /// @{

  /// \brief Gets if the current node is active in the Behavior graph
  /// \returns if the node is active
  bool IsNodeActive(const char* nodeName);

  /// \brief Sets the value of the selected Behavior float variable
  /// \param variableName  Name of the Behavior variable
  /// \param value  value we want to assign to it
  void SetFloatVar(const char* variableName, float value);

  /// \brief Sets the value of the selected Behavior word variable
  /// \param variableName  Name of the Behavior variable
  /// \param value  value we want to assign to it
  void SetWordVar(const char* variableName, int value);
  
  /// \brief Sets the value of the selected Behavior boolean variable
  /// \param variableName  Name of the Behavior variable
  /// \param value  value we want to assign to it
  void SetBoolVar(const char* variableName, bool value);
  
  /// \brief Checks the value of the selected Behavior bool variable
  /// \param variableName  Name of the Behavior variable
  bool GetBoolVar(const char* variableName) const;

  /// \brief Returns world space transform of the selected bone
  /// \param boneName  Name of the selected bone
  /// \param outPos    Bone's position
  /// \param outDir    Bone's direction vector
  void GetBoneTransform(const char* boneName, hkvVec3& outPos, hkvMat3& outRot );

  /// \brief Triggers a Behavior event
  /// \param eventName  Name of the Behavior event
  void TriggerEvent(const char* eventName) const;

  /// \brief Registers a Behavior event handler
  /// \param eventName	Name of the Behavior event
  void RegisterEventHandler(const char* eventName);

  /// \brief Checks if a given event was triggered
  /// \param eventName	Name of the Behavior event
  bool WasEventTriggered(const char* eventName) const;

  /// @}

};

#endif
