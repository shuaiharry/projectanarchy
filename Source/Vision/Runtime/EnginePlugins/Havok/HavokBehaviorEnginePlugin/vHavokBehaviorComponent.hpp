/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#ifndef HK_VISIONBEHAVIOR_COMPONENT_H
#define HK_VISIONBEHAVIOR_COMPONENT_H

#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/Havok/HavokBehaviorEnginePlugin/vHavokBehaviorResource.hpp>
#include <Behavior/Behavior/World/hkbWorldListener.h>

class hkbCharacter;
class AttachedEntity_cl;
class hkaiCharacter;
class hkaiBehavior;

/// \brief
///   Havok Behavior Component
///
class vHavokBehaviorComponent : public IVObjectComponent, public hkbWorldListener
{
	public:

		V_DECLARE_SERIAL_DLLEXP(vHavokBehaviorComponent, VHAVOKBEHAVIOR_IMPEXP);
		V_DECLARE_VARTABLE( vHavokBehaviorComponent, VHAVOKBEHAVIOR_IMPEXP )

    ///
    /// @name Constructor / Destructor
    /// @{
    ///

	  /// This ctor should only set values to null state, no actual init should go here
		VHAVOKBEHAVIOR_IMPEXP vHavokBehaviorComponent();
		VHAVOKBEHAVIOR_IMPEXP virtual ~vHavokBehaviorComponent() {}

    ///
    /// @}
    ///

		/// Cleans up the state of the blend component and prepares it for the new frame of simulation
		VHAVOKBEHAVIOR_IMPEXP void OnFrameStart();

    ///
    /// @name base class overrides
    /// @{
    ///

		virtual void SetOwner(VisTypedEngineObject_cl *pOwner) HKV_OVERRIDE;
		virtual BOOL CanAttachToObject(VisTypedEngineObject_cl *pObject, VString &sErrorMsgOut) HKV_OVERRIDE;

		virtual VVarChangeRes_e OnVariableValueChanging(VisVariable_cl *pVar, const char * value) HKV_OVERRIDE;
		virtual bool SetVariable(const char * name, const char * value) HKV_OVERRIDE;
		virtual void OnVariableValueChanged(VisVariable_cl *pVar, const char * value) HKV_OVERRIDE;
#if defined(WIN32) || defined(_VISION_DOC)
		VHAVOKBEHAVIOR_IMPEXP virtual void GetVariableAttributes(VisVariable_cl *pVariable, VVariableAttributeInfo &destInfo) HKV_OVERRIDE;
#endif

		virtual void MessageFunction(int id, INT_PTR paramA, INT_PTR paramB) HKV_OVERRIDE;

    ///
    /// @}
    ///

    ///
    /// @name 
    /// @{
    ///

    /// \brief
		///   Gets called after every Havok step. 
    ///
    /// Used to sync the Vision transforms.
		VHAVOKBEHAVIOR_IMPEXP void OnAfterHavokUpdate();

    /// \brief
		///   Update the Havok character's transform.
		VHAVOKBEHAVIOR_IMPEXP void UpdateHavokTransformFromVision();

    /// \brief
		///   Updates the collision filters on any subsystems of the character that need it.
		VHAVOKBEHAVIOR_IMPEXP void UpdateCollisionFilters( hkbCharacter* character );

    /// \brief
		///   Accessor for entity's owner.
		VHAVOKBEHAVIOR_IMPEXP inline const VisBaseEntity_cl* getEntityOwner() { return m_entityOwner; }

    /// \brief
		///   Single step the character.  
    ///
    /// Used mainly in the tool to update the pose when not simulating
		VHAVOKBEHAVIOR_IMPEXP void SingleStepCharacter();

    /// \brief
		///   Returns a normalized project path.
		void GetProjectPath(hkStringBuf& projectPath) const;

    /// \brief
		///   Set the character's resource.
		void SetResource(vHavokBehaviorResource* resource);

    ///
    /// @}
    ///

		///
		/// hkbWorldListener implementation
		///
		VOVERRIDE void eventRaisedCallback( hkbCharacter* character, const hkbEvent& event, bool raisedBySdk );

    ///
    /// @name LUA API
    /// @{
    ///

		// Returns behavior character's world-from-model transform
		VHAVOKBEHAVIOR_IMPEXP const hkQsTransform& GetWorldFromModel() const;

		// Returns whether the specified node is active
		VHAVOKBEHAVIOR_IMPEXP bool IsNodeActive(const char* nodeName);

		// Sets the value of the selected behavior float variable
		VHAVOKBEHAVIOR_IMPEXP void SetFloatVar(const char* variableName, float value);

		// Returns the value of the selected behavior float variable
		VHAVOKBEHAVIOR_IMPEXP float GetFloatVar(const char* variableName);

		// Sets the value of the selected behavior word variable
		VHAVOKBEHAVIOR_IMPEXP void SetWordVar(const char* variableName, int value);

		// Sets the value of the selected behavior float variable
		VHAVOKBEHAVIOR_IMPEXP void SetBoolVar(const char* variableName, bool value);

		// Returns world space transform of the selected bone
		VHAVOKBEHAVIOR_IMPEXP void GetBoneTransform(const char* boneName, hkvVec3& outPos, hkvMat3& outRot ) const;

		/// Initialize this character
		VHAVOKBEHAVIOR_IMPEXP void InitVisionCharacter( VisBaseEntity_cl* entityOwner );

    ///
    /// @}
    ///

		// Checks the value of the selected behavior bool variable
		VHAVOKBEHAVIOR_IMPEXP bool GetBoolVar(const char* variableName) const;

		// Triggers a behavior event
		VHAVOKBEHAVIOR_IMPEXP void TriggerEvent(const char* eventName) const;

		// Registers a behavior event handler
		VHAVOKBEHAVIOR_IMPEXP void RegisterEventHandler(const char* eventName);

		// Checks if a given event was triggered
		VHAVOKBEHAVIOR_IMPEXP bool WasEventTriggered(const char* eventName) const;

    ///
    /// @name Serialization / Resource
    /// @{
    ///

		VHAVOKBEHAVIOR_IMPEXP virtual void AssertValid() HKV_OVERRIDE;
		VHAVOKBEHAVIOR_IMPEXP virtual void Serialize(VArchive &ar) HKV_OVERRIDE;
		VHAVOKBEHAVIOR_IMPEXP virtual void OnSerialized(VArchive &ar) HKV_OVERRIDE;
#ifdef SUPPORTS_SNAPSHOT_CREATION
		VHAVOKBEHAVIOR_IMPEXP virtual void GetDependencies(VResourceSnapshot &snapshot) HKV_OVERRIDE;
#endif

    ///
    /// @}
    ///

	protected:

    /// \brief
		///   Updates the anim config and bone index list.  
    /// Usually happens after a new mesh is assigned
		void UpdateAnimationAndBoneIndexList();

    /// \brief
		///   Cleans up the component.
		void DeInit();
		
    /// \brief
		///   Update ragdoll driver and character controller driver based on
		///   values of m_enableRagdoll and m_useBehaviorWorldFromModel.
		void UpdateBehaviorPhysics();

	public:

		hkbCharacter* m_character;        ///< The Havok character

		VString m_projectPath;            ///< Path to behavior project
		VString m_projectName;            ///< Name of the behavior project
		VString m_characterName;          ///< Name of the character in the project
		VString m_behaviorName;           ///< Name of the behavior graph

		BOOL m_enableRagdoll;             ///< Whether to enable the ragdoll of the character, if one exists.
		BOOL m_useBehaviorWorldFromModel; ///< Whether to allow Behavior to modify the character's worldFromModel

	protected:

		VisBaseEntity_cl* m_entityOwner;  ///< Owner entity
	
    hkArray< int > m_boneIndexList;   ///< Bone mapping array : index is Havok, value at index is Vision.
                                      ///< -1 means no Vision bone exists for Havok bone

		VSmartPtr<vHavokBehaviorResource> m_resource; ///< Character's resource

		// An array showing which events were triggered most recently
		bool m_isListeningToEvents;
		hkArray< bool > m_triggeredEvents;

};

#endif

/*
 * Havok SDK - Base file, BUILD(#20131019)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2013
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */
